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
Command-line interface for llm_harness.
"""

import os
import argparse
from dataclasses import dataclass
from loguru import logger
from llm_harness.config import Config


@dataclass
class Arguments:
    """Command line arguments."""

    project_path: str
    model: str
    file_patterns: list[str]


def parse_arguments() -> Arguments:
    """
    Parses the command-line arguments.

    Returns:
        Arguments: The parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Generate fuzzing harnesses for C/C++ projects"
    )

    parser.add_argument(
        "project",
        help="Name of the project under the `assets/` directory, for which "
        + "harnesses are to be generated.",
    )

    parser.add_argument(
        "-m",
        "--model",
        default=Config.DEFAULT_MODEL,
        type=str,
        help=f"LLM model to be used. Available: {', '.join(Config.AVAILABLE_MODELS)}",
    )

    parser.add_argument(
        "-f",
        "--files",
        nargs="+",
        default=Config.DEFAULT_FILES,
        help="File patterns to include in analysis (e.g. *.c *.h)",
    )

    args = parser.parse_args()

    # Build the project path
    project_path = os.path.join(".", "assets", args.project)
    if not os.path.exists(project_path):
        logger.error(f"Project path does not exist: {project_path}")
        raise FileNotFoundError(f"Project path does not exist: {project_path}")

    # Validate model
    model = args.model
    if model not in Config.AVAILABLE_MODELS:
        logger.warning(
            f"Model {model} not available. Available models: "
            + f"{Config.AVAILABLE_MODELS}. "
            + f"Will use the default model ({Config.DEFAULT_MODEL})"
        )
        model = Config.DEFAULT_MODEL

    return Arguments(
        project_path=project_path, model=model, file_patterns=args.files
    )
