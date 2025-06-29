# Copyright (C) 2025 Konstantinos Chousos
#
# This file is part of OverHAuL.
#
# OverHAuL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OverHAuL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OverHAuL.  If not, see <https://www.gnu.org/licenses/>.

"""
Harness generation, fixing and improving functionality using LLMs.
"""

import json
import os
import sys
from pathlib import Path
from typing import Callable, final

import dspy
from loguru import logger

from overhaul.config import Config
from overhaul.models.project import ProjectInfo


def create_file_tools(
    base_dir: str,
) -> tuple[Callable[[str], str], Callable[[int, bool], str]]:
    """
    Factory function that creates file tools bound to a specific directory.

    Args:
        base_dir (str): The path to the project-to-be-harnessed's root.

    Returns:
        Tuple[Callable[[str], str], Callable[[int, bool], str]]: A tuple containing:
            - read_tool: Function that reads file contents within the bounded directory
            - file_index_tool: Function that lists files within the bounded directory
    """
    # Keep as Path object but don't resolve to absolute path
    base_path = Path(base_dir)
    # Get absolute path only for security checks
    base_path_abs = base_path.resolve()

    def read_tool(path: str, max_chars: int = 4000) -> str:
        """
        Reads the contents of a file within the bounded directory.

        Args:
            path (str): The relative path to the file within the bounded directory.
            max_chars (int): Maximum number of characters to return (to prevent overload).
                Defaults to 4000.

        Returns:
            str: The file content, or an error message.
        """
        logger.info(f"Reading {base_path / path}...")
        try:
            # Resolve the full path and ensure it's within base_dir
            full_path = (base_path / path).resolve()
            # Security check: ensure the resolved path is within base_dir
            if not str(full_path).startswith(str(base_path_abs)):
                return f"Error: Path {path} is outside the allowed directory"
            with open(full_path, "r", encoding="utf-8") as f:
                content = f.read(max_chars)
                if len(content) == max_chars:
                    content += "\n[...truncated]"
                return content
        except Exception as e:
            return f"Error: {str(e)}"

    def file_index_tool(
        max_files: int = 1000, include_hidden: bool = False
    ) -> str:
        """
        Returns a JSON list of files in the bounded directory tree.

        Args:
            max_files (int, optional): Maximum number of files to list. Defaults to 1000.
            include_hidden (bool, optional): Whether to include hidden files and directories. Defaults to False.

        Returns:
            str: JSON-encoded list of relative file paths, or an error message.
        """
        logger.info(f"Indexing {base_path}...")
        file_list = []
        try:
            for root, dirs, files in os.walk(base_path):
                if not include_hidden:
                    dirs[:] = [d for d in dirs if not d.startswith(".")]
                    files = [f for f in files if not f.startswith(".")]
                for file in files:
                    # Get path relative to base_dir
                    rel_path = os.path.relpath(
                        os.path.join(root, file), base_path
                    )
                    file_list.append(rel_path)
                    if len(file_list) >= max_files:
                        return json.dumps(file_list + ["...truncated"])
            return json.dumps(file_list)
        except Exception as e:
            return f"Error: {str(e)}"

    return read_tool, file_index_tool


class GenerateHarness(dspy.Signature):
    """
    You are an experienced C/C++ security testing engineer. You must write a
    libFuzzer-compatible `int LLVMFuzzerTestOneInput(const uint8_t *data, size_t
    size)` harness for a function of the given C project. Your goal is for the
    harness to be ready for compilation and for it to find successfully a bug in
    the function-under-test. Write verbose (within reason) and helpful comments
    on each step/decision you take/make.
    """

    static: str = dspy.InputField(
        desc=""" Output of static analysis tools for the project. If you find it
        helpful, write your harness so that it leverages some of the potential
        vulnerabilities described below.  """
    )
    new_harness: str = dspy.OutputField(
        desc=""" C code for a libFuzzer-compatible harness. Output only the C
        code, do not format it in a markdown code cell with backticks, so that
        it will be ready for compilation. Add **all** the necessary includes,
        either project-specific or standard libraries like <string.h>,
        <stdint.h> and <stdlib.h>. **The function to be fuzzed must be part of
        the source code**, do not write a harness for your own functions. **Do
        not truncate the input to a smaller size that the original**, e.g. for
        avoiding large stack usage or to avoid excessive buffers. Opt to using
        the heap when possible to increase the chance of exposing memory errors
        of the library, e.g. mmap instead of declaring buf[1024]. Any edge cases
        should be handled by the library itself, not the harness. On the other
        hand, do not write code that will most probably crash irregardless of
        the library under test. The point is for a function of the library under
        test to crash, not the harness itself. Use and take advantage of any
        custom structs that the library declares.  """
    )


class FixHarness(dspy.Signature):
    """
    You are an experienced C/C++ security testing engineer. Given a
    libFuzzer-compatible harness that fails to compile and its compilation
    errors, rewrite it so that it compiles successfully. Analyze the compilation
    errors carefully and find the root causes. Add any missing #includes like
    <string.h>, <stdint.h> and <stdlib.h> and #define required macros or
    constants in the fuzz target. If needed, re-declare functions or struct
    types. Add verbose comments to explain what you changed and why.
    """

    old_harness: str = dspy.InputField(desc="The harnes to be fixed.")
    error: str = dspy.InputField(desc="The compilaton error of the harness.")
    new_harness: str = dspy.OutputField(
        desc="""The newly created harness with the necessary modifications for
        correct compilation."""
    )


class ImproveHarness(dspy.Signature):
    f"""
    You are an experienced C/C++ security testing engineer. Given a
    libFuzzer-compatible harness that does not find any bug/does not crash (even
    after running for {Config.EXECUTION_TIMEOUT} seconds), you are called to
    rewrite it and improve it so that a bug can be found more easily. Determine
    the information you need to write an effective fuzz target and understand
    constraints and edge cases in the source code to do it more
    effectively. Reply only with the source code --- without backticks.
    Add verbose comments to explain what you changed and why.
    """

    old_harness: str = dspy.InputField(
        desc="The harness to be improved so it can find a bug more quickly."
    )
    output: str = dspy.InputField(desc="The output of the harness' execution.")
    new_harness: str = dspy.OutputField(
        desc="""The newly created harness with the necessary modifications for
        quicker bug-finding. If the provided harness has unnecessary input
        limitations regarding size or format etc., remove them."""
    )


@final
class Harnesser:
    """
    Given a project:
    1) Generates a libFuzzer-compatible harness for it, or
    2) Fixes an existing one that does not compile, or
    3) Improves an existing one that does not find a bug quickly enough.

    All of the above are done by an LLM using Chain of Thought prompting.
    """

    def __init__(self, model: str, project_path: str):
        """
        Initialize the harness generator.

        Args:
            model (str): The model to be used for LLM.
            project_path (str): Path to the project directory.
        """
        self.model = model

        # Ensure environment variables are loaded
        api_key = Config.load_env()
        if not api_key:
            logger.error(
                "No API key found. Make sure to set OPENAI_API_KEY in .env file."
            )
            sys.exit(-3)

        logger.info("Initializing LLM...")

        try:
            lm = dspy.LM(
                f"openai/{self.model}",
                cache=False,
                temperature=1.0,
                max_tokens=5000,
            )
            dspy.configure(lm=lm)

        except Exception as e:
            logger.error(f"Error instantiating LLM: {e}")
            raise

        read_tool, file_index_tool = create_file_tools(project_path)

        self.generator = dspy.ReAct(
            GenerateHarness, tools=[read_tool, file_index_tool]
        )
        self.fixer = dspy.ReAct(FixHarness, tools=[read_tool, file_index_tool])
        self.improver = dspy.ReAct(
            ImproveHarness, tools=[read_tool, file_index_tool]
        )

    def harness(self, project_info: ProjectInfo) -> str:
        """
        Calls the LLM to create a (new) harness for the project.

        Args:
            project_info (ProjectInfo): The project information.

        Returns:
            str: The generated harness code.
        """

        static = project_info.get_static_analysis()
        error = project_info.get_error()
        if error != None and len(error.splitlines()) > 200:
            error = "\n".join(error.splitlines()[:200]) + "\n...truncated"
        output = project_info.get_output()
        old_harness = project_info.get_harness()
        compiles = project_info.get_compilation_status()

        # Harness generation
        if not old_harness:
            logger.info("Calling LLM to generate a harness...")
            try:
                answer = self.generator(static=static)
            except Exception as e:
                logger.error(f"Error generating harness: {e}")
                raise

        # Harness fixing
        elif old_harness and not compiles:
            logger.info("Calling LLM to fix harness...")
            try:
                answer = self.fixer(
                    error=error,
                    old_harness=old_harness,
                )
            except Exception as e:
                logger.error(f"Error fixing harness: {e}")
                raise
        # Harness improving
        else:
            logger.info("Calling LLM to improve harness...")
            try:
                answer = self.improver(
                    output=output,
                    old_harness=old_harness,
                )
            except Exception as e:
                logger.error(f"Error improving harness: {e}")
                raise

        project_info.harness = answer.new_harness
        return str(answer.new_harness)
