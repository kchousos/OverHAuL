# Copyright (C) 2025 Konstantinos Chousos
#
# This file is part of LLM-Harness.
#
# LLM-Harness is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# LLM-Harness is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LLM-Harness.  If not, see <https://www.gnu.org/licenses/>.

"""
Harness generation, fixing and improving functionality using LLMs.
"""

import sys
import dspy
from loguru import logger
from typing import final
from llm_harness.models.project import ProjectInfo
from llm_harness.config import Config


class GenerateHarness(dspy.Signature):
    """
    You are an experienced C/C++ security testing engineer. You must write a
    libFuzzer-compatible `int LLVMFuzzerTestOneInput(const uint8_t *data, size_t
    size)` harness for a function of the given C project. Your goal is for the
    harness to be ready for compilation and for it to find successfully a bug in
    the function-under-test.
    """

    source: str = dspy.InputField(
        desc="The source files of the project, concatenated."
    )
    readme: str = dspy.InputField(desc="The README of the project.")
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
        <stdint.h> and <stdlib.h>. The function to be fuzzed must be part of the
        source code. Do not limit in any way the input or its size for any
        reason, e.g. avoiding large stack usage. Any edge cases should be
        handled by the library itself, not the harness. On the other hand, do
        not write code that will most probably crash irregardless of the library
        under test. The point is for a function of the library under test to
        crash, not the harness itself. Use and take advantage of any custom
        structs that the library declares.  """
    )


class FixHarness(dspy.Signature):
    """
    You are an experienced C/C++ security testing engineer. Given a
    libFuzzer-compatible harness that fails to compile and its compilation
    errors, rewrite it so that it compiles successfully. Analyze the compilation
    errors carefully and find the root causes. Add any missing #includes like
    <string.h>, <stdint.h> and <stdlib.h> and #define required macros or
    constants in the fuzz target. If needed, re-declare functions or struct
    types.
    """

    source: str = dspy.InputField(
        desc="The source files of the project, concatenated."
    )
    old_harness: str = dspy.InputField(desc="The harnes to be fixed.")
    error: str = dspy.InputField(desc="The compilaton error of the harness.")
    new_harness: str = dspy.OutputField(
        desc="The newly created harness with the necessary modifications for \
        correct compilation."
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
    """

    source: str = dspy.InputField(
        desc="The source files of the project, concatenated."
    )
    old_harness: str = dspy.InputField(
        desc="The harness to be improved so it can find a bug more quickly."
    )
    output: str = dspy.InputField(desc="The output of the harness' execution.")
    new_harness: str = dspy.OutputField(
        desc="The newly created harness with the necessary modifications for \
        quicker bug-finding."
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

    def __init__(self, model: str):
        """
        Initialize the harness generator.

        Args:
            model (str): The model to be used for LLM.
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

        self.generator = dspy.ChainOfThought(GenerateHarness)
        self.fixer = dspy.ChainOfThought(FixHarness)
        self.improver = dspy.ChainOfThought(ImproveHarness)

    def harness(self, project_info: ProjectInfo) -> str:
        """
        Calls the LLM to create a (new) harness for the project.

        Args:
            project_info (ProjectInfo): The project information.

        Returns:
            str: The generated harness code.
        """

        source = project_info.get_source()
        static = project_info.get_static_analysis()
        readme = project_info.get_readme()
        error = project_info.get_error()
        output = project_info.get_output()
        old_harness = project_info.get_harness()
        compiles = project_info.get_compilation_status()

        # Harness generation
        if not old_harness:
            logger.info("Calling LLM to generate a harness...")
            try:
                answer = self.generator(
                    source=source, readme=readme, static=static
                )
            except Exception as e:
                logger.error(f"Error generating harness: {e}")
                raise

        # Harness fixing
        elif old_harness and not compiles:
            logger.info("Calling LLM to fix harness...")
            try:
                answer = self.fixer(
                    source=source,
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
                    source=source,
                    output=output,
                    old_harness=old_harness,
                )
            except Exception as e:
                logger.error(f"Error improving harness: {e}")
                raise

        project_info.harness = answer.new_harness
        return answer.new_harness
