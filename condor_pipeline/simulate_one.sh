#!/bin/bash
set -euo pipefail

if [[ $# -ne 4 ]]; then
  echo "Usage: simulate_one.sh INPUT OUTPUT_PREFIX EXECUTABLE PARAMETERS" >&2
  exit 2
fi

input=$1
output_prefix=$2
optics_executable=$3
parameters=$4

[[ -s "$input" ]] || { echo "Missing converted input: $input" >&2; exit 1; }
[[ -x "$optics_executable" ]] || { echo "Missing optics executable: $optics_executable" >&2; exit 1; }
[[ -s "$parameters" ]] || { echo "Missing parameters: $parameters" >&2; exit 1; }

mkdir -p "$(dirname "$output_prefix")"
job_id="${_CONDOR_CLUSTER:-local}.${_CONDOR_PROC:-$$}"
temporary_prefix="${output_prefix}.tmp.${job_id}"
cleanup() {
  rm -f "${temporary_prefix}_feb" "${temporary_prefix}_seb" \
        "${temporary_prefix}_fhb" "${temporary_prefix}_shb" \
        "${temporary_prefix}_A_sums2"
}
trap cleanup EXIT

# Existing optical tables and config paths are relative to TAIGA_optics.
cd "$(dirname "$parameters")"
"$optics_executable" \
  --input-file "$input" \
  --output-prefix "$temporary_prefix" \
  --parameters "$parameters"

product=""
for suffix in _feb _seb _fhb _shb; do
  candidate="${temporary_prefix}${suffix}"
  if [[ -s "$candidate" ]]; then
    product=$candidate
    mv -f "$candidate" "${output_prefix}${suffix}"
    break
  fi
done
[[ -n "$product" ]] || { echo "TAIGA_optics produced no known output" >&2; exit 1; }

if [[ -f "${temporary_prefix}_A_sums2" ]]; then
  mv -f "${temporary_prefix}_A_sums2" "${output_prefix}_A_sums2"
fi
touch "${output_prefix}.done"
trap - EXIT
echo "Simulated: $input"
