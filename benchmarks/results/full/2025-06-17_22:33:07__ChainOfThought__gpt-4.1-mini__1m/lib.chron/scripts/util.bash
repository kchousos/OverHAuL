set -o errexit

RED='\033[0;31m'
GREEN='\033[1;32m'
YELLOW='\033[0;33m'

DEFAULT='\033[0m'

red () {
  printf "${RED}$@${DEFAULT}"
}

green () {
  printf "${GREEN}$@${DEFAULT}"
}

yellow () {
  printf "${YELLOW}$@${DEFAULT}"
}

for_each () {
  local fn=$1

  shift

  local -a arr=($@)

  for item in "${arr[@]}"; do
    $fn $item
  done
}

filter () {
  local fn=$1
  local arg

  while read -r arg; do
    $fn $arg && echo $arg
  done
}
