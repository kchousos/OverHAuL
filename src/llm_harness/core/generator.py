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
from llm_harness.models.project import ProjectInfo
from llm_harness.config import Config


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
        try:
            lm = dspy.LM(f"openai/{self.model}", cache=False)
            dspy.configure(lm=lm)

            concatenated_content = project_info.get_concatenated_content()

            response = lm(
                f"""
                I have this C project, for which you will find the contents below.
                Write me a fuzzing harness for the dateparse function. Respond **only**
                with the harness' code. Make sure to write all the necessary includes
                etc. The harness will be located in a `harnesses/` subdirectory from
                the project root, so make sure the includes work appropriately.

                Do not even wrap the code in markdown fences, e.g. ```, because it
                will be automatically written to a .c file.

                === Source Code ===

                {concatenated_content}
                """
            )

            return response[0]
        except Exception as e:
            logger.error(f"Error creating harness: {e}")
            raise
