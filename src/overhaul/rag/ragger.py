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
RAG functionality for the project's chunked code.
"""

from typing import Any, Callable

import faiss
import numpy as np
from loguru import logger


def search_vector_store(
    index: faiss.Index,
    meta: list[dict[str, Any]],
    query: str,
    embedder: Any,
    topk: int = 3,
) -> list[dict[str, Any]]:
    """
    Perform a semantic similarity search over the vector store to find the most relevant code chunks for a given query.

    Args:
        index (faiss.Index): FAISS index containing code embeddings.
        meta (list[dict[str, Any]]): Metadata list mapping embedding indices to code information (code, signature, filepath).
        query (str): Query string to search for relevant functions.
        embedder (Any): Embedder instance with an `embed` method to convert query to an embedding vector.
        topk (int, optional): Number of top similar results to return. Defaults to 3.

    Returns:
        list[dict[str, Any]]: List of metadata dicts for the top-k most relevant code chunks.
    """
    qvec = np.array(embedder([query]), dtype=np.float32)
    if qvec.ndim == 1:
        qvec = qvec.reshape(1, -1)
    _, Ids = index.search(qvec, topk)
    results = []
    for idx in Ids[0]:
        if idx != -1:
            results.append(meta[idx])
    return results


def make_rag_tool(
    index: faiss.Index,
    meta: list[dict[str, Any]],
    embedder: Any,
) -> Callable[[str, int], str]:
    """
    Constructs a retrieval-augmented generation (RAG) tool that can be used as a callable to retrieve relevant code snippets by semantic similarity.

    Args:
        index (faiss.Index): FAISS index containing code embeddings.
        meta (list[dict[str, Any]]): Metadata list mapping embedding indices to code information (code, signature, filepath).
        embedder (Any): Embedder instance with an `embed` method to convert queries to embedding vectors.

    Returns:
        Callable[[str, int], str]: A retrieval tool function taking a question and top-k value, returning formatted relevant code chunks.
    """

    def rag_tool(question: str, topk: int = 5) -> str:
        """
        Retrieve the most relevant code snippets for a given question.

        Args:
            question (str): Query string for which relevant code is to be fetched.
            topk (int): Number of top relevant code chunks to return.

        Returns:
            str: Concatenated string of the most relevant code chunks, including their filepaths and signatures.
        """
        logger.debug(f'Agent using RAG: "{question}"')
        relevant = search_vector_store(
            index, meta, question, embedder, topk=topk
        )
        if not relevant:
            answer = "No relevant code found."
        else:
            answer = "\n\n".join(
                (
                    f"File: {item['filepath']}\n"
                    f"Signature: {item['signature']}\n"
                    f"Code:\n{item['code']}"
                )
                for item in relevant
            )
        return answer

    return rag_tool
