<h1 align="center">OverHAuL: Harness Automation with LLMs</h1>

<div align="center">

Transform your code security — one harness at a time.

<p>
<a href="https://www.repostatus.org/#active"><img src="https://www.repostatus.org/badges/latest/active.svg" alt="Project Status: Active – The project has reached a stable, usable state and is being actively developed." /></a>
<img
src="https://img.shields.io/badge/Python-%3E%3D%0A3.10-3776AB.svg?logo=python&amp;logoColor=white"
alt="python" /> <img
src="https://img.shields.io/github/license/kchousos/overhaul"
alt="GitHub License" /> <img
src="https://img.shields.io/github/actions/workflow/status/kchousos/overhaul/tests.yml?label=tests"
alt="GitHub Actions Workflow Status" /> <img
src="https://img.shields.io/coverallsCoverage/github/kchousos/overhaul?branch=master"
alt="Coveralls" />
<!-- <a href="https://docs.astral.sh/ruff/"> -->
<!-- <img src="https://img.shields.io/badge/code%20formatter-ruff-d7ff64" -->
<!-- alt="code formatter: ruff" /></a>  -->
<!-- <a href="http://mypy-lang.org/"><img -->
<!-- src="https://img.shields.io/badge/type%20check-mypy-blue" -->
<!-- alt="type check: mypy" /></a> -->
</p>

</div>

**OverHAuL** (*Harness Automation with LLMs*) is a system for automatically generating libFuzzer-compatible harnesses for C projects using ReAct-style LLM agents. It iteratively builds harnesses tailored for effective fuzzing through tool-assisted reasoning — all without manual intervention. OverHAuL's goal is to make fuzzing accessible and immediate, reducing friction and accelerating the path to safer software.

## Installation

### Dependencies

- Python >=3.10
- [uv](https://docs.astral.sh/uv/)

### Installation Steps

1. Clone the repository and cd into it:

    ```bash
    git clone https://github.com/kchousos/overhaul.git;
    cd overhaul
    ```

2. Create a virtual environment (optional):

    ```bash
    python3.10 -m venv .venv;
    source .venv/bin/activate # for bash
    ```

3. Install the project:

    ```bash
    uv pip install .
    ```

## Usage

1. Add an OpenAI API key in `.env`, such as:

    ```bash
    # cat .env
    OPENAI_API_KEY=<API-key-here>
    ```
    
    Or export it as an environment variable:

    ```bash
    export OPENAI_API_KEY=<API-key-here>
    ```

2. Execute the main script:

    ```bash
    overhaul <repo-link>
    ```

The cloned repo with the newly generated harness can be found in the `output/` directory.

### Command-Line Options

```
$ overhaul --help
usage: overhaul [-h] [-c COMMIT] [-m MODEL] [-f FILES [FILES ...]] [-o OUTPUT_DIR] repo

Generate fuzzing harnesses for C/C++ projects

positional arguments:
  repo                  Link of a project's git repo, for which to generate a harness.

options:
  -h, --help            show this help message and exit
  -c COMMIT, --commit COMMIT
                        A specific commit of the project to check out
  -m MODEL, --model MODEL
                        LLM model to be used. Available: o3-mini, o3, gpt-4o, gpt-4o-mini, gpt-4.1, gpt-4.1-mini
  -f FILES [FILES ...], --files FILES [FILES ...]
                        File patterns to include in analysis (e.g. *.c *.h)
  -o OUTPUT_DIR, --output-dir OUTPUT_DIR
                        Directory to clone the project into. Defaults to output
```
