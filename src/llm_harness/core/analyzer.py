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

import os
import sys
import glob
import subprocess
import shutil
from typing import List, Optional
from loguru import logger
from llm_harness.models.project import ProjectFile, ProjectInfo
from llm_harness.config import Config


class ProjectAnalyzer:
    """
    Analyzes project files and extracts relevant information.
    """

    def __init__(
        self, project_path: str, file_patterns: Optional[List[str]] = None
    ):
        """
        Initialize the project analyzer.

        Args:
            project_path (str): Path to the project directory.
            file_patterns (List[str], optional): File patterns to include.
        """
        self.project_path = project_path
        self.file_patterns = file_patterns or Config.DEFAULT_FILES

    def collect_source_code(self) -> ProjectInfo:
        """
        Collects project's source code.

        Returns:
            ProjectInfo: Information about the project.
        """
        logger.info("Reading project and collecting information...")

        files: list[ProjectFile] = []

        project_files = self._find_project_files()
        if not project_files:
            logger.error("No project files found!")
            return ProjectInfo(source=files)

        for file_path in project_files:
            try:
                with open(file_path, "r", encoding="utf-8") as f:
                    files.append(
                        ProjectFile(
                            path=file_path,
                            name=os.path.basename(file_path),
                            content=f.read(),
                        )
                    )
            except IOError as e:
                logger.error(f"Error reading file {file_path}: {e}")

        if not files:
            logger.error("No project files found!")

        return ProjectInfo(source=files)

    def _find_project_files(self) -> List[str]:
        """
        Finds all project files matching the specified patterns.

        Returns:
            List[str]: List of file paths.
        """
        all_files = []
        for pattern in self.file_patterns:
            matched_files = glob.glob(os.path.join(self.project_path, pattern))
            all_files.extend(matched_files)

        return all_files

    def get_static_analysis(self, backend: str = "cppcheck") -> str:
        if backend == "cppcheck":
            if shutil.which(backend) is None:
                logger.error("cppcheck not in path. Exiting...")
                sys.exit(1)
            static = subprocess.run(
                [
                    "cppcheck",
                    "--enable=warning,performance,information,unusedFunction,missingInclude",
                    "--std=c++17",
                    self.project_path,
                    "-i",
                    Config.HARNESS_DIR,
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )

            return static.stdout

        return ""

    def collect_project_info(self) -> ProjectInfo:
        """
        Collects information about the project by reading files.

        Returns:
            ProjectInfo: Information about the project.
        """
        info = self.collect_source_code()
        info.static = self.get_static_analysis()
        return info
