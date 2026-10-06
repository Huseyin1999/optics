#!/bin/bash
set -euo pipefail
export LD_LIBRARY_PATH="$PWD:${LD_LIBRARY_PATH:-}"
hostname
./hybrid 10_iact.corsika 10_iact_split 100
test -s 10_iact_split_i
test -s 10_iact_split_t
ls -lh 10_iact_split_i 10_iact_split_t
