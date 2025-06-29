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
File operations for the overhaul package.
"""

import os
from typing import final

from loguru import logger

from overhaul.config import Config


@final
class FileManager:
    """
    Handles file operations for the harness generator.
    """

    def __init__(self, project_path: str) -> None:
        """
        Initialize the file manager.

        Args:
            project_path (str): Path to the project directory.

        Returns:
            None
        """
        self.project_path = project_path
        self.harness_dir = os.path.join(project_path, Config.HARNESS_DIR)

    def write_harness(self, harness: str, filename: str = "") -> None:
        """
        Writes the harness to the harnesses directory.

        Args:
            harness (str): The harness code to write.
            filename (str, optional): The filename to use.

        Returns:
            None
        """
        logger.info("Writing harness to project...")
        os.makedirs(self.harness_dir, exist_ok=True)

        if filename == "":
            filename = Config.HARNESS_FILENAME

        harness_path = os.path.join(self.harness_dir, filename)

        # Check if the file already exists
        if os.path.exists(harness_path):
            # Determine the base name and the extension
            base, ext = os.path.splitext(filename)
            i = 1

            # Create new name with _<n> suffix
            new_filename = f"{base}_{i}{ext}"
            new_path = os.path.join(self.harness_dir, new_filename)

            # Increment if the numbered file already exists
            while os.path.exists(new_path):
                i += 1
                new_filename = f"{base}_{i}{ext}"
                new_path = os.path.join(self.harness_dir, new_filename)

            # Rename the existing file to the new name
            os.rename(harness_path, new_path)
            logger.info(f"Existing file renamed to {new_path}")

        # Write the new harness code to the original path
        try:
            with open(harness_path, "w", encoding="utf-8") as f:
                f.write(harness)
            logger.info(f"Harness written to {harness_path}")
        except IOError as e:
            logger.error(f"Error writing harness to {harness_path}: {e}")
            raise
