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
Builds the generated harness.
"""

import os
import subprocess
from loguru import logger
from typing import Optional
from llm_harness.config import Config


class HarnessBuilder:
    """
    Builds a project's generated harness.
    """

    def __init__(self, project_path: str):
        """
        Initialize the builder.

        Args:
            project_path (str): Path to the project directory.
        """
        self.project_path = project_path
        self.cc = Config().CC
        self.cflags = Config().CFLAGS
        self.executable = Config().EXECUTABLE_FILENAME
        self.harness_dir = Config().HARNESS_DIR

    def build_harness(
        self, harness_filename: Optional[str] = Config().HARNESS_FILENAME
    ) -> tuple[str, bool]:
        """
        Builds the LLM-generated harness.

        Args:
            filename (Optional[str]): Name of the harness file. Defaults to `harness.c`.

        Returns:
            str: Build output including success or error message.
        """
        logger.info("Building harness...")

        if not harness_filename:
            harness_filename = Config().HARNESS_FILENAME

        harness_filename = os.path.join(self.harness_dir, harness_filename)

        # Collect source files recursively
        source_files = [harness_filename]
        for _, _, files in os.walk(self.project_path):
            for f in files:
                if f != os.path.basename(harness_filename) and f.endswith(
                    ".c"
                ):
                    source_files.append(f)

        compilation_command = [
            self.cc,
            *self.cflags,
            *source_files,
            "-I.",
            "-o",
            self.executable,
        ]

        logger.info(f"Starting compilation of harness: {harness_filename}")
        try:
            completed_process = subprocess.run(
                compilation_command,
                check=True,
                capture_output=True,
                text=True,
                cwd=self.project_path,
            )
            logger.info("Harness compiled successfully")
            return completed_process.stdout, True

        except subprocess.CalledProcessError as e:
            logger.error("Error during harness compilation")
            logger.error(
                f"Standard Output:\n{e.stdout}\nStandard Error:\n{e.stderr}"
            )
            return f"Error {e.returncode}: {e.stderr}", False
