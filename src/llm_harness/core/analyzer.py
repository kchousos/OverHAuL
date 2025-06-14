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
from loguru import logger
from llm_harness.models.project import ProjectFile, ProjectInfo
from llm_harness.config import Config


class ProjectAnalyzer:
    """
    Analyzes project files and extracts relevant information.
    """

    def __init__(self, project_path: str, file_patterns: list[str] = []):
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
            sys.exit(-1)

        for file_path in project_files:
            if os.path.basename(file_path) in Config.IGNORED_FILES:
                continue
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

    def _find_project_files(self) -> list[str]:
        """
        Finds all project files matching the specified patterns.

        Returns:
            list[str]: List of file paths.
        """
        all_files = []
        for subdir in Config.DEFAULT_DIRS:
            search_root = os.path.join(self.project_path, subdir)
            for pattern in self.file_patterns:
                search_pattern = os.path.join(search_root, pattern)
                matched_files = glob.glob(search_pattern)
                all_files.extend(matched_files)

        return all_files

    def get_static_analysis(
        self, backends: list[str] = ["cppcheck", "flawfinder"]
    ) -> str:
        """
        Concatenates and returns the output of the static analysis backends
        specified in `backends`.

        Args:
            backends (list[str], optional): A list of backends to use for static
                analysis. Defaults to CPPCheck and Flawfinder.

        Returns:
            str: The concatenated output of all the enabled static analysis backends.

        """
        static = ""

        if "cppcheck" in backends:
            if shutil.which("cppcheck") is None:
                logger.error("cppcheck not in path. Exiting...")
                sys.exit(1)

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

            c_files = glob.glob("*.c", root_dir=self.project_path)

            output = subprocess.run(
                ["flawfinder"] + c_files,
                cwd=self.project_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )

            static += output.stdout

        return static

    def collect_project_info(self) -> ProjectInfo:
        """
        Collects information about the project by reading files.

        Returns:
            ProjectInfo: Information about the project.
        """
        info = self.collect_source_code()
        info.static = self.get_static_analysis()
        return info
