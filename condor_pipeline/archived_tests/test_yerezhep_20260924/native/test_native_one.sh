#!/bin/bash
set -euo pipefail
export LD_LIBRARY_PATH="$PWD:${LD_LIBRARY_PATH:-}"
hostname
cat /etc/os-release
ldd ./TAIGA_optics_file
date -u
time bash simulate_one.sh 10_iact_split_i assets parameters.txt
date -u
ls -lh result.bin result_A_sums2 result.done
