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
Builds the generated harness.
"""

import fnmatch
import os
import subprocess
from typing import final

from loguru import logger

from overhaul.config import Config


@final
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
        self.cc = Config.CC
        self.cflags = Config.CFLAGS
        self.executable = Config.EXECUTABLE_FILENAME
        self.harness_dir = Config.HARNESS_DIR

    def build_harness(
        self, harness_filename: str | None = Config.HARNESS_FILENAME
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
            harness_filename = Config.HARNESS_FILENAME

        harness_filename = os.path.join(self.harness_dir, harness_filename)

        # Collect source files recursively
        source_files = [harness_filename]

        for root, dirs, files in os.walk(self.project_path):
            # Modify dirs in-place to skip hidden directories
            dirs[:] = [d for d in dirs if not d.startswith(".")]

            for f in files:
                if f.startswith(".") or f.startswith("harness"):
                    continue  # Skip hidden files and past harness attempts
                full_path = os.path.join(root, f)
                dir_components = root.split(os.sep)

                if (
                    not any(
                        fnmatch.fnmatch(f, pattern)
                        for pattern in Config.IGNORED_FILES
                    )
                    and f != os.path.basename(harness_filename)
                    and f.endswith(".c")
                    and not any(
                        ignored in dir_components
                        for ignored in Config.IGNORED_DIRS
                    )
                ):  # Get the relative path of the file from project_path
                    full_path = os.path.relpath(
                        os.path.join(root, f), start=self.project_path
                    )
                    source_files.append(full_path)

        # Collect Include directories recursively
        include_dirs = set(Config.DEFAULT_DIRS)
        for root, _, files in os.walk(self.project_path):
            if not any(
                part.startswith(".")
                for part in os.path.relpath(root, self.project_path).split(
                    os.sep
                )
            ):
                include_dirs.add(
                    os.path.relpath(
                        os.path.join(root), start=self.project_path
                    )
                )

        # Convert to include flags
        include_flags = [item for d in include_dirs for item in ("-I", d)]

        compilation_command = [
            self.cc,
            *self.cflags,
            *source_files,
            *include_flags,
            "-o",
            self.executable,
        ]

        # Save compilation command as a build script
        script_path = (
            f"{self.project_path}/{Config.COMPILATION_SCRIPT_FILENAME}"
        )

        # Write to the file
        with open(script_path, "w") as file:
            file.write("#!/bin/bash\n")  # Shebang line for shell script
            file.write(" ".join(compilation_command) + "\n")

        # Make the script executable
        os.chmod(script_path, 0o755)

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
            logger.warning("Error during harness compilation")
            return f"Error {e.returncode}: {e.stderr}", False
