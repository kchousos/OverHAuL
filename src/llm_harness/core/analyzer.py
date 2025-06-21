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
Project analysis functionality.
"""

import fnmatch
import glob
import os
import shutil
import subprocess
import sys

from loguru import logger

from llm_harness.config import Config
from llm_harness.models.project import ProjectInfo


class ProjectAnalyzer:
    """
    Analyzes project files and extracts relevant information.
    """

    def __init__(self, project_path: str):
        """
        Initialize the project analyzer.

        Args:
            project_path (str): Path to the project directory.
        """
        self.project_path = project_path

    def get_static_analysis(self, backends: list[str] = ["flawfinder"]) -> str:
        """
        Concatenates and returns the output of the static analysis backends
        specified in `backends`.

        Args:
            backends (list[str], optional): A list of backends to use for
            static analysis. Defaults to Flawfinder. Available options: CPPCheck
            and Flawfinder.

        Returns:
            str: The concatenated output of all the enabled static analysis backends.
        """
        static = ""

        if "cppcheck" in backends:
            if shutil.which("cppcheck") is None:
                logger.warning("cppcheck not in path. Skipping...")
                return "CPPCheck not available."

            static += "=== CPPCheck output ===\n\n"
            output = subprocess.run(
                [
                    "cppcheck",
                    "--enable=warning,performance,information,unusedFunction,missingInclude",
                    "--std=c++17",
                    "--suppress=missingIncludeSystem",
                    "--suppress=unusedFunction",
                    "--check-level=exhaustive",
                    "-i",
                    Config.HARNESS_DIR,
                    ".",
                ],
                cwd=self.project_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )

            static += output.stdout

        if "flawfinder" in backends:
            static += "=== Flawfinder output ===\n\n"

            output = subprocess.run(
                ["flawfinder", "."],
                cwd=self.project_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )

            static += output.stdout

        return static

    def collect_project_info(self) -> ProjectInfo:
        """
        Collects information about the project.

        Right now, the only info that is provided by default is the project's
        static analysis.

        Returns:
            ProjectInfo: Information about the project.
        """

        info = ProjectInfo()
        info.static = self.get_static_analysis()
        return info
