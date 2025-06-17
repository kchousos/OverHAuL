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
Harness generation functionality.
"""

import sys
import dspy
from loguru import logger
from typing import final
from llm_harness.models.project import ProjectInfo
from llm_harness.config import Config


class GenerateHarness(dspy.Signature):
    """
    Generate a libFuzzer-compatible harness for the given C project that is
    ready for compilation and finds succesfully a bug in the project.
    """

    source: str = dspy.InputField(
        desc="The source files of the project, concatenated."
    )
    readme: str = dspy.InputField(desc="The README of the project.")
    static: str = dspy.InputField(
        desc="Output of static analysis tools for the project"
    )
    harness: str = dspy.OutputField(
        desc="C code for a libFuzzer-compatible harness with **all** the necessary\
        includes, either project-specific or standard libraries like <string.h>,\
        <stdint.h> and <stdlib.h>, that will be ready for compilation. The function\
        to be fuzzed must be part of the source code. Do not limit in any way the\
        input to the library or format it in a specific way or cap its length for\
        any reason.These edge cases should be handled by the library itself, not\
        the harness. Output only the C code, do not format it in a markdown code\
        cell with backticks."
    )


@final
class HarnessGenerator:
    """
    Generates a libFuzzer-compatible harness for the given project using an
    LLM and Chain of Thought prompting.
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

    def create_harness(self, project_info: ProjectInfo) -> str:
        """
        Calls the LLM to create a harness for the project.

        Args:
            project_info (ProjectInfo): The project information.

        Returns:
            str: The generated harness code.
        """
        logger.info("Calling LLM to generate a harness...")

        try:
            lm = dspy.LM(
                f"openai/{self.model}",
                cache=False,
                temperature=1.0,
                max_tokens=5000,
            )
            dspy.configure(lm=lm)

            source = project_info.get_source()
            static = project_info.get_static_analysis()
            readme = project_info.get_readme()

            harnesser = dspy.ChainOfThought(GenerateHarness)

            answer = harnesser(source=source, readme=readme, static=static)

            return answer.harness

        except Exception as e:
            logger.error(f"Error creating harness: {e}")
            raise


class FixHarness(dspy.Signature):
    """
    Fix the harness provided, given its compilation errors.
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


@final
class HarnessFixer:
    """
    Given a libFuzzer-compatible harness and its compilation error, uses an LLM
    with Chain of Thought prompting to correct it so that it compiles succesfully.
    """

    def __init__(self, model: str):
        """
        Initialize the harness fixer.

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

    def fix_harness(self, project_info: ProjectInfo) -> str:
        """
        Calls the LLM to fix the harness of the project.

        Args:
            project_info (ProjectInfo): The project information.

        Returns:
            str: The fixed harness code.
        """
        logger.info("Calling LLM to fix harness...")

        try:
            # lm = dspy.LM(
            #     f"openai/{self.model}",
            #     cache=False,
            #     temperature=1.0,
            #     max_tokens=5000,
            # )
            # dspy.configure(lm=lm)

            source = project_info.get_source()
            error = project_info.get_error()
            old_harness = project_info.get_harness()

            harnesser = dspy.ChainOfThought(FixHarness)

            answer = harnesser(
                source=source, error=error, old_harness=old_harness
            )

            return answer.new_harness

        except Exception as e:
            logger.error(f"Error fixing harness: {e}")
            raise
