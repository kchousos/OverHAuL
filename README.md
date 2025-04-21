<h1 align="center">LLM-Harness</h1>

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
alt="Coveralls" /></p>

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
- [uv](https://docs.astral.sh/uv/)

    ```bash
    pipx install uv
    ```

### Installation Steps

1. Clone the repository:

    ```bash
    git clone https://github.com/kchousos/llm-harness.git
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
    uv run python main.py <repo-name>
    ```

### Command-Line Options

```
$ python main.py --help
usage: main.py [-h] [-m MODEL] [-f FILES [FILES ...]] project

Generate fuzzing harnesses for C/C++ projects

positional arguments:
  project               Name of the project under the `assets/` directory, for which harnesses are to be generated.

options:
  -h, --help            show this help message and exit
  -m MODEL, --model MODEL
                        LLM model to be used. Available: gpt-4.1-mini, o4-mini, o3-mini, gpt-4o, gpt-4o-mini
  -f FILES [FILES ...], --files FILES [FILES ...]
                        File patterns to include in analysis (e.g. *.c *.h)
```