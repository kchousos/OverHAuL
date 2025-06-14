#!/bin/bash

# Default values
MODEL=""

log() {
    local level="$1"
    shift

    local color_reset='\033[0m'
    local color=""
    local timestamp

    case "$level" in
        INFO) color='\033[0;36m' ;;  # cyan
        SUCCESS)  color='\033[0;32m' ;;  # green
        WARN)  color='\033[0;33m' ;;  # yellow
        ERROR) color='\033[0;31m' ;;  # red
        *)
            # No recognized level: print message only and return
            echo "$level $*"
            return
            ;;
    esac

    timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    echo -e "${color}${timestamp} [$level]${color_reset} $*"
}

# Function to run on Ctrl-C
cleanup() {
    log INFO "Interrupted by user (Ctrl-C). Exiting..."
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
        -h|--help)
            echo "Usage: $0 [-m|--model <model_name>]"
            exit 1
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

# Counters for total repos and failures
total=0
failures=0
new_crashes=0

log INFO "Starting benchmark..."

# Read each line of input file
while IFS= read -r line || [[ -n "$line" ]]; do
    # Skip empty lines or lines starting with #
    [[ -z "$line" || "$line" =~ ^# ]] && continue

    ((total++))

    # Split line into repo and optional commit hash
    repo=$(echo "$line" | awk '{print $1}')
    commit=$(echo "$line" | awk '{print $2}')

    # Build base command
    cmd=(llm-harness "$repo")

    # Add commit if present
    if [[ -n "$commit" ]]; then
        cmd+=(-c "$commit")
    fi

    # Add model if specified
    if [[ -n "$MODEL" ]]; then
        cmd+=(-m "$MODEL")
    fi

    # Run command hiding output
    # Remove trailing slash if any
    repo_clean="${repo%/}"

    # Extract the user/repo portion:

    if [[ "$repo_clean" =~ github\.com[:/](.+/.+)(\.git)?$ ]]; then
        repo_name="${BASH_REMATCH[1]}"
    else
        # fallback, just basename if no match
        repo_name=$(basename "$repo_clean")
        repo_name="${repo_name%.git}"
    fi

    # run llm-harness
    "${cmd[@]}" > /dev/null 2>&1
    status=$?

    # Check command exit status and count failures
    if [[ $status -eq 0 ]]; then
        log SUCCESS "$repo_name harnessed"
        ((new_crashes++))
    elif [[ $status -eq 254 ]]; then
        log WARN "The generated harness for $repo_name didn't produce a crash"
    elif [[ $status -eq 255 ]]; then
        log ERROR "The generated harness for $repo_name did not compile correctly"
        ((failures++))
    fi

done < "$INPUT_FILE"

# Print summary
log INFO "Benchmark complete"
echo
log INFO "$new_crashes projects harnessed succesfully"
log INFO "$((total - new_crashes)) harnesses did not find a new crash"
log INFO "$failures harnesses failed to compile"
log INFO "$total projects total"
