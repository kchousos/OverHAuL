#!/usr/bin/env bash
IFS=$'\n'

TESTING_DIR=t
UTIL_F=util.bash
REPO_DIR=chron

declare -a SKIP_FILES=(

)

not_test_file () {
	local test=$1

	for (( i=0; i < ${#SKIP_FILES[@]}; i++ )); do
		if [[ $test == ${SKIP_FILES[i]} ]]; then
			return 1
		else
			return 0
		fi
	done
}

run_test () {
	local file_name="$1"

	gcc -Isrc -Ideps -c "$TESTING_DIR/$file_name" -o main.o
	gcc -o main main.o -L./ -l $REPO_DIR

	export LD_LIBRARY_PATH=$HOME/repositories/$REPO_DIR/src/:$LD_LIBRARY_PATH
	green "\n[+] Running test...\n\n"

	./main
}

main () {
	make

	declare -a tests=($(ls $TESTING_DIR | filter not_test_file))

	for_each run_test ${tests[*]}
}

. "$(dirname "$(readlink -f "$BASH_SOURCE")")"/$UTIL_F
main $*
