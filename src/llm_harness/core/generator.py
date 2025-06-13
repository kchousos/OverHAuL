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

import dspy
from loguru import logger
from typing import final
from llm_harness.models.project import ProjectInfo
from llm_harness.config import Config


@final
class HarnessGenerator:
    """
    Generates a harness for a project using an LLM.
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
            logger.warning(
                "No API key found. Make sure to set OPENAI_API_KEY in .env file."
            )

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
                top_p=0.95,
            )
            dspy.configure(lm=lm)

            concatenated_content = project_info.get_concatenated_content()
            static_analysis = project_info.get_static_analysis()

            response = lm(
                f"""
                I have this C project, for which you will find the contents
                below. Write me a libFuzzer-compatible harness for it.
                Respond **only** with the harness' code. Make sure to write *all
                the necessary includes* etc. The harness will be located in the
                project root, so make sure the includes work appropriately.
                Take into account each file's path.

                Try to write a new harness and not default to one from your
                training dataset. It will be a .c file.

                Do not even wrap the code in markdown fences, e.g. ```, because
                it will be automatically written to a .c file.

                The function to be fuzzed **must** be part of the source code!

                Select one function to fuzz at random.

                Before generating the harness, think step by step about what
                function might be interesting or nontrivial to fuzz.  Avoid
                choosing the same kind of target as in previous attempts.

                **Do not add arbitrary checks for the input, like limiting input
                size, or worrying about stack usage. These things must be
                accounted by the fuzzed library, this is why I need the harness.
                On the other hand, do not write code that will most probably
                crash, i.e. non-null terminated strings etc. The point is for a
                function of the library under test to crash, not the harness
                itself.**

                The point is to catch the Program Under Test "by surprise", so
                do not format your input to make its job easier,
                e.g. NULL-terminated strings or using specific keywords to the
                program to help it.

                Use and take advantage of any custom structs that the library
                declares.

                Again, Make sure to add **all the necessary includes**, like
                e.g. <string.h>, <stdint.h> or <stdlib.h> if they are needed.

                === Source Code ===

                {concatenated_content}
                
                What follows is static analysis output. If you find it helpful,
                write your harness so that it leverages some of the potential
                vulnerabilities described below.

                === Static Analysis Output ===
                {static_analysis}
                """
            )

            return str(response[0])
        except Exception as e:
            logger.error(f"Error creating harness: {e}")
            raise
