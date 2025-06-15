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
Configuration management for the llm_harness package.
"""

import os
from dataclasses import dataclass
from typing import final
from dotenv import load_dotenv


@final
@dataclass
class Config:
    """Configuration for llm_harness."""

    # List of available models
    AVAILABLE_MODELS = [
        # "o4-mini",
        # "o3-mini",
        # "o3",
        # "o3-pro",
        "o1",
        "o1-pro",
        "gpt-4o",
        "gpt-4o-mini",
        "gpt-4.1",
        "gpt-4.1-mini",
    ]

    # Default model if none provided
    DEFAULT_MODEL = "gpt-4.1-mini"

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
    DEFAULT_FILES = ["*.c", "*.h", "*.h.in"]

    # Files to be ignored
    IGNORED_FILES = [
        "*test.c",
        "*unit.c",
        "main.c",
        "benchmark.c",
        "benchmarks.c",
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

    # Harness directory name
    # Defaults to project's root directory
    HARNESS_DIR = "harnesses"

    # Harness default filename
    HARNESS_FILENAME = "harness.c"

    # Default C compilation options
    CC = "clang"
    CFLAGS = [
        "-g",
        "-fsanitize=fuzzer,address,undefined",
    ]  # needed for fuzzing
    EXECUTABLE_FILENAME = "harness"

    MIN_EXECUTION_TIME = 5  # minutes

    EXECUTION_TIMEOUT = 20  # seconds

    @staticmethod
    def load_env() -> str | None:
        """Load environment variables from .env file."""
        load_dotenv()
        return os.environ.get("OPENAI_API_KEY")
