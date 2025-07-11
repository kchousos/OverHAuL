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
Runs and evaluates the generated harness.
"""

import os
import subprocess
import time
from typing import final

from loguru import logger

from overhaul.config import Config


@final
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

    def _list_crash_files(self) -> set[tuple[str, float]]:
        """
        List all crash-* files in the given directory along with their creation time.

        Returns:
            Set[Tuple[str, float]]: A set of tuples (filename, creation_time).
        """
        return {
            (f, os.path.getctime(os.path.join(self.project_path, f)))
            for f in os.listdir(self.project_path)
            if f.startswith("crash-")
        }

    def _list_leak_files(self) -> set[tuple[str, float]]:
        """
        List all leak-* files in the given directory along with their creation time.

        Returns:
            Set[Tuple[str, float]]: A set of tuples (filename, creation_time).
        """
        return {
            (f, os.path.getctime(os.path.join(self.project_path, f)))
            for f in os.listdir(self.project_path)
            if f.startswith("leak-")
        }

    def evaulate_harness(self) -> tuple[str, bool]:
        """
        Runs and evaluates the LLM-generated harness.

        Returns:
            bool: Returns whether the harness "passes" the evaluation.
        """
        logger.info("Evaluating harness...")

        execution_command = f"./{self.executable}"

        before_crashes = self._list_crash_files()
        before_leaks = self._list_leak_files()

        harness_output = ""

        logger.info("Starting execution of harness...")

        start_time = time.time()

        try:
            captured_output = subprocess.run(
                execution_command,
                check=False,
                capture_output=True,
                cwd=self.project_path,
                timeout=Config().EXECUTION_TIMEOUT,
            )
            end_time = time.time()
            harness_output = captured_output.stderr.decode(
                "utf-8", errors="replace"
            )
        except subprocess.TimeoutExpired as e:
            logger.warning(
                f"Execution timed out after {Config().EXECUTION_TIMEOUT} seconds."
            )
            return f"Error: {str(e.stderr)}", False

        # save harness output
        with open(
            os.path.join(self.project_path, Config.HARNESS_OUTPUT), "w"
        ) as f:
            f.write(harness_output)

        runtime = end_time - start_time
        logger.info(f"Harness execution completed in {runtime:.2f} seconds.")

        after_crashes = self._list_crash_files()
        testcases = after_crashes - before_crashes

        empty = False
        if len(testcases) > 0:
            # newest testcase
            case, _ = max(
                testcases, key=lambda x: x[1]
            )  # x[0] is filename, x[1] is ctime
            result = subprocess.run(
                ["xxd", case],
                capture_output=True,
                text=True,
                cwd=self.project_path,
            )
            if result.stdout.strip() == "":
                empty = True

        after_leaks = self._list_leak_files()
        leaks = after_leaks - before_leaks

        leak = None
        leak_content = None
        if len(leaks) > 0:
            # newest leak
            leak, _ = max(
                leaks, key=lambda x: x[1]
            )  # x[0] is filename, x[1] is ctime
            leak_content = subprocess.run(
                ["xxd", leak],
                capture_output=True,
                text=True,
                cwd=self.project_path,
            )
            if leak_content.stdout.strip() == "":
                leak = None

        # Leak file was generated
        if leak:
            error = f"Leak file was generated with content: {str(leak_content)}.\n\n"
            logger.warning(
                f"Leak file was generated with content: {str(leak_content)}."
            )
            return error + harness_output, False
        # Check if new testcases were created
        elif len(testcases) == 0:
            error = "No new testcases were generated.\n\n"
            logger.warning("No new testcases were generated.")
            return error + harness_output, False
        # Check if testcase is valid
        elif empty:
            error = "Testcase is invalid (empty xxd output).\n\n"
            logger.warning("Testcase is invalid (empty xxd output).")
            return error + harness_output, False
        # Check runtime
        elif runtime < Config().MIN_EXECUTION_TIME and len(testcases) == 0:
            error = "Harness does not execute correctly.\n\n"
            logger.warning("Harness does not execute correctly.")
            return error + harness_output, False
        else:
            logger.info(
                f"New testcases created ({len(testcases)}): {testcases}"
            )

        return harness_output, True
