<h1 align="center">LLM-Harness (TBD)</h1>

<div align="center">

Use LLMs to automatically generate fuzzing harnesses for your
C/C++ project.
    
<p><a href="https://www.repostatus.org/#wip"><img
src="https://www.repostatus.org/badges/latest/wip.svg"
alt="Project Status: WIP – Initial development is in progress, but there has not yet been a stable, usable release suitable for the public." /></a>
<img
src="https://img.shields.io/badge/Python-%3E%3D%0A3.10-3776AB.svg?logo=python&amp;logoColor=white"
alt="python" /> <img
src="https://img.shields.io/github/license/kchousos/llm-harness"
alt="GitHub License" /> <img
src="https://img.shields.io/github/actions/workflow/status/kchousos/llm-harness/tests.yml?label=tests"
alt="GitHub Actions Workflow Status" /> <img
src="https://img.shields.io/coverallsCoverage/github/kchousos/llm-harness?branch=master"
alt="Coveralls" />
<a href="https://docs.astral.sh/ruff/">
<img src="https://img.shields.io/badge/code%20formatter-ruff-d7ff64"
alt="code formatter: ruff" /></a> 
<a href="http://mypy-lang.org/"><img
src="https://img.shields.io/badge/type%20check-mypy-blue"
alt="type check: mypy" /></a></p>

</div>

- Collects information of your project structure and files.
- Gives relevant context to LLM.
- Automatically writes generated harness.
- Builds any generated harness and evaluates it.
- Supports OpenAI's models.

# Getting Started

## Installation

### Dependencies

- Python >=3.10
- [Cppcheck](https://github.com/danmar/cppcheck)
- [uv](https://docs.astral.sh/uv/)

### Installation Steps

1. Clone the repository and cd into it:

    ```bash
    git clone https://github.com/kchousos/llm-harness.git;
    cd llm-harness
    ```

2. Install the project:

    ```bash
    uv sync
    ```

## Usage

1. Add an OpenAI API key in `.env`, such as:

    ```bash
    # cat .env
    OPENAI_API_KEY=<API-key-here>
    ```
2. Execute the main script:

    ```bash
    uv run python main.py <repo-link>
    ```

### Command-Line Options

```
$ uv run python main.py --help
usage: main.py [-h] [-c COMMIT] [-m MODEL] [-f FILES [FILES ...]] repo

Generate fuzzing harnesses for C/C++ projects

positional arguments:
  repo                  Link of a project's git repo, for which to generate a harness.

options:
  -h, --help            show this help message and exit
  -c COMMIT, --commit COMMIT
                        A specific commit of the project to check out
  -m MODEL, --model MODEL
                        LLM model to be used. Available: gpt-4.1-mini, o1, o1-pro, gpt-4o, gpt-4o-mini, gpt-4.1, gpt-4.1-mini
  -f FILES [FILES ...], --files FILES [FILES ...]
                        File patterns to include in analysis (e.g. *.c *.h)
```
