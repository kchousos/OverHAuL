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

from typing import Callable, final

import dspy
from loguru import logger

from overhaul.config import Config
from overhaul.models.project import ProjectInfo


class GenerateHarness(dspy.Signature):
    """
    You are an experienced C/C++ security testing engineer. You must write a
    libFuzzer-compatible `int LLVMFuzzerTestOneInput(const uint8_t *data, size_t
    size)` harness for a function of the given C project. Your goal is for the
    harness to be ready for compilation and for it to find successfully a bug in
    the function-under-test. Write verbose (within reason) and helpful comments
    on each step/decision you take/make, especially if you use "weird" constants
    or values that have something to do with the project.

    You have access to a rag_tool, which contains a vector store of
    function-level chunks of the project. Use it to write better harnesses. Keep
    in mind that it can only reply with function chunks, do not ask it to
    combine stuff.

    The rag_tool does not store any information on which lines the functions
    are. So do not ask questions based on lines.

    Make sure that you only fuzz an existing function. You will know that a
    functions exists when the rag_tool returns to you its signature and body.
    """

    static: str = dspy.InputField(
        desc=""" Output of static analysis tools for the project. If you find it
        helpful, write your harness so that it leverages some of the potential
        vulnerabilities described below.  """
    )
    new_harness: str = dspy.OutputField(
        desc=""" C code for a libFuzzer-compatible harness. Output only the C
        code, **DO NOT format it in a markdown code cell with backticks**, so
        that it will be ready for compilation.

        <important>
        
        Add **all** the necessary includes, either project-specific or standard
        libraries like <string.h>, <stdint.h> and <stdlib.h>. Also include any
        header files that are part of the project and are probably useful. Most
        projects have a header file with the same name as the project at the
        root.

        **The function to be fuzzed absolutely must be part of the source
        code**, do not write a harness for your own functions or speculate about
        existing ones. You must be sure that the function that is fuzzed exists
        in the source code. Use your rag tool to query the source code.

        Do not try to fuzz functions of the project that are static, since they
        are only visible in the file that they were declared. Choose other
        user-facing functions instead.

        </important>

        **Do not truncate the input to a smaller size that the original**,
        e.g. for avoiding large stack usage or to avoid excessive buffers. Opt
        to using the heap when possible to increase the chance of exposing
        memory errors of the library, e.g. mmap instead of declaring
        buf[1024]. Any edge cases should be handled by the library itself, not
        the harness. On the other hand, do not write code that will most
        probably crash irregardless of the library under test. The point is for
        a function of the library under test to crash, not the harness
        itself. Use and take advantage of any custom structs that the library
        declares.

        Do not copy function declarations inside the harness. The harness will
        be compiled in the root directory of the project.  """
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

    old_harness: str = dspy.InputField(desc="The harness to be fixed.")
    error: str = dspy.InputField(desc="The compilaton error of the harness.")
    new_harness: str = dspy.OutputField(
        desc="""The newly created harness with the necessary modifications for
        correct compilation."""
    )


class ImproveHarness(dspy.Signature):
    f"""
    You are an experienced C/C++ security testing engineer. Given a
    libFuzzer-compatible harness that does not find any bug/does not crash (even
    after running for {Config.EXECUTION_TIMEOUT} seconds) or has memory leaks
    (generates leak files), you are called to rewrite it and improve it so that
    a bug can be found more easily and/or memory is managed correctly. Determine
    the information you need to write an effective fuzz target and understand
    constraints and edge cases in the source code to do it more
    effectively. Reply only with the source code --- without backticks.  Add
    verbose comments to explain what you changed and why.
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

    def __init__(
        self,
        model: str,
        project_path: str,
        rag_tool: Callable[[str, int], str],
    ):
        """
        Initialize the harness generator.

        Args:
            model (str): The model to be used for LLM.
            project_path (str): Path to the project directory.
        """
        self.model = model

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

        self.generator = dspy.ReAct(GenerateHarness, tools=[rag_tool])
        self.fixer = dspy.ReAct(FixHarness, tools=[rag_tool])
        self.improver = dspy.ReAct(ImproveHarness, tools=[rag_tool])

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
        if error is not None and len(error.splitlines()) > 200:
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
