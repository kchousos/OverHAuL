#!/bin/bash

# Fail on errors
# set -e

# Default values
MODEL=""

# Function to run on Ctrl-C
cleanup() {
    echo -e "\nInterrupted by user (Ctrl-C). Exiting..."
    exit 130  # 130 is standard exit code for script terminated by Ctrl-C
}

# Trap SIGINT (Ctrl-C)
trap cleanup SIGINT

# Parse script arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--model)
            MODEL="$2"
            shift 2
            ;;
        *)
            echo "Usage: $0 [-m|--model <model_name>]"
            exit 1
            ;;
    esac
done

# Change to project root (assumes script is in benchmarks/)
cd "$(dirname "$0")/.."

INPUT_FILE="benchmarks/repos.txt"

while IFS= read -r line || [[ -n "$line" ]]; do
    [[ -z "$line" || "$line" =~ ^# ]] && continue

    repo=$(echo "$line" | awk '{print $1}')
    commit=$(echo "$line" | awk '{print $2}')

    # Build base command
    cmd=(uv run python main.py "$repo")

    # Add commit if exists
    if [[ -n "$commit" ]]; then
        cmd+=(-c "$commit")
    fi

    # Add model if specified
    if [[ -n "$MODEL" ]]; then
        cmd+=(-m "$MODEL")
    fi

    "${cmd[@]}"

done < "$INPUT_FILE"
