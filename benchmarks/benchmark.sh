#!/bin/bash


# Benchmarking Script for LLM-Harness
#
# This script is intended for the evaluation of LLM-Harness.
# It is not part of the project’s core implementation and should not be considered production code.
#
# This script reads repo links from ./repos.txt and runs LLM-Harness on them.
# The result provides a count summary of:
#     1. Projects where a crash was found.
#     2. Projects where the harness compiled succesfully but did not find a crash.
#     3. Projects where the harness did not compile.
#
# Note: Results may vary depending on the LLM model used and its variance in the harness generation.
#
# Usage:
#     ./benchmark.sh
#
# Author: Konstantinos Chousos


# Default values
MODEL=""
SECONDS=0
BENCHMARK="mini"

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
        -b|--benchmark)
            BENCHMARK="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [-m|--model <model_name>] [-b|--benchmark [mini|full]]"
            exit 1
            ;;
        *)
            echo "Usage: $0 [-m|--model <model_name>] [-b|--benchmark [mini|full]]"
            exit 1
            ;;
    esac
done

# Change to project root (assumes script is in benchmarks/)
cd "$(dirname "$0")/.."

# Counters for total repos and failures
total=0
failures=0
new_crashes=0

model=""
if [[ -n "$MODEL" ]]; then
    model="$MODEL"
else
    model="gpt-4.1-mini"
fi

benchmark=""
if [[ "$BENCHMARK" == "full" ]]; then
    benchmark="benchmarks/repos.txt"
else
    benchmark="benchmarks/repos-mini.txt"
fi

timestamp=$(date +"%Y-%m-%d_%H:%M:%S")
module="ChainOfThought"
timeout="1"
basename="${BENCHMARK}/${timestamp}__${module}__${model}__${timeout}m"
OUT_DIR="benchmarks/results/${basename}"
LOG_FILE="results.log"
mkdir -p "$OUT_DIR"

{

log INFO "Benchmark used: $BENCHMARK"
log INFO "LLM model used: $model"
log INFO "Max duration for harness execution: ${timeout} min"
log INFO "Prompting technique (DSPy module): $module"
echo 
echo "==================== Benchmark start ======================="
echo 

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

    cmd+=(-o "$OUT_DIR")

    log INFO "Generating harness for $repo_name..."

    # run llm-harness
    "${cmd[@]}" > /dev/null 2>&1
    status=$?

    # Check command exit status and count failures
    if [[ $status -eq 0 ]]; then
        log SUCCESS "Harness successful"
        ((new_crashes++))
    elif [[ $status -eq 254 ]]; then
        log WARN "The generated harness didn't produce a crash"
    elif [[ $status -eq 255 ]]; then
        log ERROR "The generated harness did not compile correctly"
        ((failures++))
    fi

done < "$benchmark"

# Print summary
echo 
echo "==================== Benchmark complete ===================="
echo
log INFO "$new_crashes projects harnessed succesfully"
log INFO "$((total - new_crashes - failures)) harnesses did not find a new crash"
log INFO "$failures harnesses failed to compile"
log INFO "$total projects total"
hours=$((SECONDS / 3600))
minutes=$(( (SECONDS % 3600) / 60 ))
seconds=$((SECONDS % 60))
log INFO "Total runtime: ${hours}h ${minutes}m ${seconds}s"

} | tee /dev/tty | sed -e 's/\x1B\[0;3[0-9]m//g' -e 's/\x1B\[0m//g' >> "$LOG_FILE"

mv $LOG_FILE $OUT_DIR
