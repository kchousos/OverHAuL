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
import re
import sys
import shutil
import subprocess
import argparse
from dataclasses import dataclass
from loguru import logger
from llm_harness.config import Config
from llm_harness.core.analyzer import ProjectAnalyzer
from llm_harness.core.generator import Harnesser
from llm_harness.core.builder import HarnessBuilder
from llm_harness.core.evaluator import HarnessEvaluator
from llm_harness.io.file_manager import FileManager


@dataclass
class Arguments:
    """Command line arguments."""

    project_path: str
    model: str
    file_patterns: list[str]


def get_repo_name(url: str) -> str:
    """
    Gets a repo's name from its GitHub link.

    Arguments:
        url (str): The link to the repo.

    Returns:
        str: The repo's project name.

    Raises:
        ValueError: If the URL does not contain a valid repo name.
    """
    match = re.search(r"/([^/]+?)(?:\.git)?/?$", url)
    if match:
        return match.group(1)
    raise ValueError(f"Could not extract repo name from URL: {url}")


def shallow_clone(
    repo_url: str, destination: str = ".", commit: str | None = None
) -> None:
    """
    Clone a Git repository with depth 1 and optionally checkout a specific commit.
    Also initializes and updates submodules.
    After cloning and all operations, deletes the .git directory.

    Args:
        repo_url (str): The repository URL.
        destination (str): Target directory to clone into.
        commit (str, optional): Commit hash to checkout after cloning.
    """
    if os.path.exists(destination):
        shutil.rmtree(destination)

    subprocess.run(
        ["git", "clone", "--depth", "1", repo_url, destination],
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    if commit:
        subprocess.run(
            ["git", "fetch", "--depth", "1", "origin", commit],
            check=True,
            cwd=destination,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        subprocess.run(
            ["git", "checkout", commit],
            check=True,
            cwd=destination,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    subprocess.run(
        [
            "git",
            "submodule",
            "update",
            "--init",
            "--recursive",
            "--depth",
            "1",
        ],
        check=True,
        cwd=destination,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    git_dir = os.path.join(destination, ".git")
    if os.path.exists(git_dir):
        shutil.rmtree(git_dir)

    github_dir = os.path.join(destination, ".github")
    if os.path.exists(github_dir):
        shutil.rmtree(github_dir)


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
        "repo",
        help="Link of a project's git repo, for which to generate a harness.",
    )

    parser.add_argument(
        "-c",
        "--commit",
        required=False,
        type=str,
        help="A specific commit of the project to check out",
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

    parser.add_argument(
        "-o",
        "--output-dir",
        default=Config.DEFAULT_CLONE_DIR,
        type=str,
        help=f"Directory to clone the project into. Defaults to {Config.DEFAULT_CLONE_DIR}",
    )

    args = parser.parse_args()

    # Clone repo under the project's name
    project_path = get_repo_name(args.repo)
    project_path = os.path.join(args.output_dir, project_path)
    logger.info(f"Cloning project's repo in the {project_path} directory...")
    shallow_clone(args.repo, project_path, args.commit)

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


def main() -> int:
    """
    Main entry point of the application. Collects project info, calls
    LLM to create and write a harness for the project.

    Returns:
        bool: Whether the harness is up to par to be merged to the project.
    """
    # 1. Get user options
    args = parse_arguments()
    project_path, model = args.project_path, args.model

    # 2. Read project source and analyze it
    analyzer = ProjectAnalyzer(project_path)
    project_info = analyzer.collect_project_info()

    harnesser = Harnesser(model=model)
    file_manager = FileManager(project_path)
    builder = HarnessBuilder(project_path)
    evaluator = HarnessEvaluator(project_path)

    acceptable = False

    # 3. Harnessing feedback loop
    for i in range(Config.MAX_ITERATIONS):
        logger.info(f"Iteration {i + 1} of harnessing...")

        # 3a. Create/update harness
        harness = harnesser.harness(project_info=project_info)

        # 3b. Integrate harness to project
        file_manager.write_harness(harness)

        # 3c. Build harness
        error, compiled = builder.build_harness()
        project_info.compiles = compiled
        # 3c1. If harness does not compile correctly, regenerate it
        if not compiled:
            logger.warning("Could not compile harness. Reiterating...")
            project_info.error = error
            continue  # Go to 3a again

        # 3d. Run and evaluate harness
        output, accepted = evaluator.evaulate_harness()
        # 3d1. If harness does not pass evaluation, regenerate it
        if not accepted:
            logger.warning("Harness does not pass evaluation. Reiterating...")
            project_info.output = output
            continue  # Go to 3a again
        else:
            acceptable = True
            break

    # 4. Summary
    compiles = project_info.get_compilation_status()
    if not compiles:
        logger.error("Could not generate a compilable harness. Exiting...")
        sys.exit(-1)
    elif not acceptable:
        logger.error(
            "The generated harness did not pass the evaluation. Exiting..."
        )
        sys.exit(-2)

    logger.info("All done!")
    sys.exit(0)


if __name__ == "__main__":
    main()
