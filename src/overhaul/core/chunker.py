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
Project chunking through clang and RAG functionality.
"""

import fnmatch
import os
from typing import Any

from clang import cindex

# from dspy import Embedder
# from loguru import logger
from overhaul.config import Config


def extract_functions_with_metadata(project_path: str) -> list[Any]:
    """
    Parses all non-ignored files in project and extracts function-level chunks using clang.

    Args:
        project_path (str): The path to the project.

    Returns:
        List[dict]: List of chunk dicts with keys 'code', 'signature', 'filepath'.
    """

    cindex.Config.set_library_file("/usr/lib64/libclang.so")

    chunks = []
    for root, dirs, files in os.walk(project_path):
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
                and f.endswith(tuple(Config.DEFAULT_EXTENSIONS))
                and not any(
                    ignored in dir_components
                    for ignored in Config.IGNORED_DIRS
                )
            ):
                index = cindex.Index.create()
                tu = index.parse(full_path)

                if not tu or not tu.cursor:
                    continue  # Skip files that failed to parse

                for node in tu.cursor.walk_preorder():
                    if (
                        node.kind == cindex.CursorKind.FUNCTION_DECL
                        and node.is_definition()
                    ):
                        extent = node.extent
                        with open(full_path) as file:
                            lines = file.readlines()
                            chunk = "".join(
                                lines[extent.start.line - 1 : extent.end.line]
                            )
                        signature = node.type.spelling
                        chunks.append(
                            {
                                "code": chunk,
                                "signature": signature,
                                "filepath": full_path,
                            }
                        )
    return chunks
