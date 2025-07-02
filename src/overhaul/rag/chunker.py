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
Project chunking through clang.
"""

import fnmatch
import os
from typing import Any

import dspy
import faiss
import numpy as np
from clang import cindex

from overhaul.config import Config


def extract_chunks(project_path: str) -> list[dict[str, str]]:
    """
    Parses all non-ignored source files in the project and extracts function-level chunks using clang.

    Args:
        project_path (str): Path to the root of the project.

    Returns:
        List[dict]: Each dict has keys: 'code', 'signature', 'filepath'
    """
    # Limit: At most this many chars per chunk (well under OpenAI's context limit).
    MAX_CHARS = 4000

    cindex.Config.set_library_file("/usr/lib64/libclang.so")

    chunks = []
    for root, dirs, files in os.walk(project_path):
        # Skip hidden directories
        dirs[:] = [d for d in dirs if not d.startswith(".")]

        for f in files:
            if f.startswith(".") or f.startswith("harness"):
                continue  # Skip hidden or generated files
            full_path = os.path.join(root, f)
            dir_components = root.split(os.sep)

            # Only process non-ignored C source/header files
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
                try:
                    tu = index.parse(full_path)
                except Exception as e:
                    print(f" * Failed to parse {full_path}: {e}")
                    continue
                if not tu or not tu.cursor:
                    continue  # Skip files that can't be parsed

                for node in tu.cursor.walk_preorder():
                    if (
                        node.kind == cindex.CursorKind.FUNCTION_DECL
                        and node.is_definition()
                    ):
                        extent = node.extent
                        try:
                            with open(full_path, "r") as file:
                                lines = file.readlines()
                        except Exception as e:
                            print(f" * Failed to read {full_path}: {e}")
                            continue
                        chunk = "".join(
                            lines[extent.start.line - 1 : extent.end.line]
                        )
                        # Filter out empty and excessively large chunks
                        if not chunk.strip() or len(chunk) > MAX_CHARS:
                            continue
                        signature = node.type.spelling
                        chunks.append(
                            {
                                "code": chunk,
                                "signature": signature,
                                "filepath": "/".join(
                                    full_path.split("/", 1)[
                                        1:
                                    ]  # remove 'output/' directory
                                ),
                            }
                        )
    return chunks


def embed_chunks(
    chunks: list[dict[str, str]],
    embedder: dspy.Embedder,
) -> tuple[faiss.IndexIDMap, list[dict[str, Any]]]:
    """Compute embeddings and build a FAISS ID-mapped index with metadata.

    Args:
        chunks (List[Dict[str, str]]): Each dict has 'code', 'signature', 'filepath'.
        embedder: DSPy Embedder instance.

    Returns:
        Tuple[faiss.IndexIDMap, List[Dict[str, Any]]]:
            - faiss index: faiss.IndexIDMap, mapping ids to embeddings
            - meta: List of metadata dicts. meta[idx] corresponds to id=idx.
    """
    valid_chunks = []
    for i, c in enumerate(chunks):
        code = c.get("code", "")
        if isinstance(code, str) and code.strip():
            valid_chunks.append(c)

    codes = [c["code"] for c in valid_chunks]
    embeddings = np.array(embedder(codes), dtype=np.float32)
    if embeddings.ndim == 1:
        embeddings = embeddings.reshape(1, -1)
    embeddings = np.ascontiguousarray(embeddings)

    dim = embeddings.shape[1]

    # Use unique IDs (we'll just use the index in meta as the id)
    ids = np.arange(len(chunks)).astype(np.int64)

    # Build base index and wrap with ID map
    base_index = faiss.IndexFlatL2(dim)
    index = faiss.IndexIDMap(base_index)
    index.add_with_ids(embeddings, ids)

    meta: list[dict[str, Any]] = [
        {
            "code": chunk["code"],
            "signature": chunk["signature"],
            "filepath": chunk["filepath"],
        }
        for chunk in chunks
    ]

    return index, meta
