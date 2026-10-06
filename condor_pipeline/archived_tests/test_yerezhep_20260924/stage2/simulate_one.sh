#!/bin/bash
set -euo pipefail
[[ $# -eq 3 ]] || exit 2
scratch=$PWD
test -s "$scratch/$1"
cd "$2"
"$scratch/TAIGA_optics_file" --input-file "$scratch/$1" --output-prefix "$scratch/result" --parameters "$3"
count=0
for suffix in _feb _seb _fhb _shb; do
  if [[ -s "$scratch/result$suffix" ]]; then
    mv "$scratch/result$suffix" "$scratch/result.bin"
    count=$((count+1))
  fi
done
[[ $count -eq 1 ]] || exit 1
test -f "$scratch/result_A_sums2"
touch "$scratch/result.done"
