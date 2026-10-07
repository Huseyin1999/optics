#!/bin/bash
set -euo pipefail
[[ $# -eq 2 ]] || exit 2
tar -xzf runtime.tar.gz
export LD_LIBRARY_PATH="$PWD/runtime/lib:${LD_LIBRARY_PATH:-}"
test -s "$1"
./runtime/bin/hybrid "$1" result "$2"
test -s result_i
test -s result_t
touch result.done
