#!/bin/bash
set -euo pipefail

if [[ $# -ne 5 ]]; then
  echo "Usage: convert_one.sh INPUT OUTPUT_PREFIX HYBRID RADIUS CORSIKA_DIR" >&2
  exit 2
fi

input=$1
output_prefix=$2
hybrid=$3
radius=$4
corsika_dir=$5

[[ -s "$input" ]] || { echo "Missing input: $input" >&2; exit 1; }
[[ -x "$hybrid" ]] || { echo "Missing executable hybrid: $hybrid" >&2; exit 1; }
[[ -d "$corsika_dir" ]] || { echo "Missing CORSIKA directory: $corsika_dir" >&2; exit 1; }

mkdir -p "$(dirname "$output_prefix")"
export CORSIKADIR="$corsika_dir"
export LD_LIBRARY_PATH="$(dirname "$hybrid")/lib:${LD_LIBRARY_PATH:-}"

job_id="${_CONDOR_CLUSTER:-local}.${_CONDOR_PROC:-$$}"
temporary_prefix="${output_prefix}.tmp.${job_id}"
cleanup() {
  rm -f "${temporary_prefix}_t" "${temporary_prefix}_i"
}
trap cleanup EXIT

"$hybrid" "$input" "$temporary_prefix" "$radius"
[[ -s "${temporary_prefix}_t" ]] || { echo "hybrid did not create _t output" >&2; exit 1; }
[[ -s "${temporary_prefix}_i" ]] || { echo "hybrid did not create _i output" >&2; exit 1; }

mv -f "${temporary_prefix}_t" "${output_prefix}_t"
mv -f "${temporary_prefix}_i" "${output_prefix}_i"
trap - EXIT
echo "Converted: $input"
