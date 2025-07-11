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
Configuration management for the overhaul package.
"""

import os
from dataclasses import dataclass
from typing import final

from dotenv import load_dotenv


@final
@dataclass
class Config:
    """Configuration for overhaul."""

    # List of available models
    AVAILABLE_MODELS = [
        "o3-mini",
        "o3",
        "gpt-4o",
        "gpt-4o-mini",
        "gpt-4.1",
        "gpt-4.1-mini",
        "gpt-3.5-turbo",
        "gpt-4",
    ]

    # Default model if none provided
    DEFAULT_MODEL = "gpt-4.1-mini"

    EMBEDDING_MODEL = "text-embedding-3-small"

    DEFAULT_DIRS = [
        ".",
        "src",
        "source",
        "sources",
        "include",
        "lib",
        "deps",
        "dependencies",
        "contrib",
    ]

    # Default files to include if none specified
    DEFAULT_FILES = ["*.c", "*.h"]
    DEFAULT_EXTENSIONS = [".c", ".h"]

    # File regexps to be ignored
    IGNORED_FILES = [
        "*test.c",
        "*unit.c",
        "main.c",
        "*benchmark*.c",
        "*example*.c",
    ]
    IGNORED_DIRS = [
        "test",
        "tests",
        "example",
        "examples",
        "demo",
        "demos",
        "benchmark",
        "benchmarks",
    ]

    DEFAULT_CLONE_DIR = "output"

    # Harness directory name
    HARNESS_DIR = "harnesses"

    # Harness default filename
    HARNESS_FILENAME = "harness.c"

    HARNESS_OUTPUT = "harness.out"

    COMPILATION_SCRIPT_FILENAME = "overhaul.sh"

    # Default C compilation options
    CC = "clang"
    CFLAGS = [
        "-g3",
        "-fsanitize=fuzzer,address,undefined,leak",
    ]  # needed for fuzzing
    EXECUTABLE_FILENAME = "harness"

    MIN_EXECUTION_TIME = 5 * 60  # seconds

    EXECUTION_TIMEOUT = 5 * 60  # seconds

    # Max iterations for the feedback loop between LLM and compilation/harness output
    MAX_ITERATIONS = 10

    @staticmethod
    def load_env() -> str | None:
        """Load environment variables from .env file."""
        load_dotenv()
        return os.environ.get("OPENAI_API_KEY")
