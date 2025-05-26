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
Runs and evaluates the generated harness.
"""

import subprocess
import time
import os
from loguru import logger
from llm_harness.config import Config


class HarnessEvaluator:
    """
    Runs and evaluates a project's generated harness.
    """

    def __init__(self, project_path: str):
        """
        Initialize the evaluator.

        Args:
            project_path (str): Path to the project directory.
        """
        self.project_path = project_path
        self.executable = Config().EXECUTABLE_FILENAME

    def _list_crash_files(self) -> set[str]:
        """
        List all crash-* files in the given directory.

        Args:
            directory (str): The directory to search for crash files.
        """
        return {
            f for f in os.listdir(self.project_path) if f.startswith("crash-")
        }

    def evaulate_harness(self) -> bool:
        """
        Runs and evaluates the LLM-generated harness.

        Returns:
            bool: Returns whether the harness "passes" the evaluation.
        """
        logger.info("Evaluating harness...")

        execution_command = f"./{self.executable}"

        before = self._list_crash_files()

        logger.info("Starting execution of harness...")

        start_time = time.time()

        try:
            subprocess.run(
                execution_command,
                stdout=subprocess.DEVNULL,
                check=False,
                cwd=self.project_path,
                timeout=Config().EXECUTION_TIMEOUT,
            )
            end_time = time.time()
        except subprocess.TimeoutExpired:
            logger.error(
                f"Execution timed out after {Config().EXECUTION_TIMEOUT} seconds."
            )
            return False

        runtime = end_time - start_time
        logger.info(f"Harness execution completed in {runtime:.2f} seconds.")

        after = self._list_crash_files()
        testcases = after - before

        # Check if new testcases were created
        if len(testcases) == 0:
            logger.error("No new testcases were generated.")
            return False
        else:
            logger.info(
                f"New testcases created ({len(testcases)}): {testcases}"
            )

        # Minimum runtime
        if runtime / 60 < Config().MIN_EXECUTION_TIME and testcases is None:
            logger.error("Harness does not execute correctly.")
            return False

        return True
