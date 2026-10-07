#!/bin/bash
set -euo pipefail
[[ $# -eq 5 ]] || { echo "Expected input radius parameters suffix job_key" >&2; exit 2; }
scratch=$PWD
input=$1
radius=$2
parameters=$3
suffix=$4
job_key=$5
echo "START $(date -u +%FT%TZ) host=$(hostname)"
tar -xzf runtime.tar.gz
export LD_LIBRARY_PATH="$scratch/runtime/lib:${LD_LIBRARY_PATH:-}"
test -s "$input"
echo "CONVERT $(date -u +%FT%TZ)"
./runtime/bin/hybrid "$input" intermediate "$radius"
test -s intermediate_i
test -s intermediate_t
echo "OPTICS $(date -u +%FT%TZ)"
cd runtime/assets
"$scratch/runtime/bin/TAIGA_optics_file" --input-file "$scratch/intermediate_i" --output-prefix "$scratch/result" --parameters "$parameters"
cd "$scratch"
test -s "result$suffix"
test -s result_A_sums2
mv "result$suffix" result.bin
printf '%s\n' "$job_key" > result.done
echo "FINISH $(date -u +%FT%TZ)"
ls -lh result.bin result_A_sums2
