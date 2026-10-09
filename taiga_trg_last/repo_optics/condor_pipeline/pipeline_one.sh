#!/bin/bash
set -euo pipefail
[[ $# -ge 5 ]] || { echo "Usage: pipeline_one.sh input radius parameters suffix job_key [keep_feb]" >&2; exit 2; }

scratch=$PWD
input=$1
radius=$2
parameters=$3
suffix=$4
job_key=$5
keep_feb=${6:-0}

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

rm -f intermediate_i intermediate_t
cp "result$suffix" result.bin
tar -czf result_trg.tar.gz -T /dev/null

if [[ -f runtime/bin/trigger_iact ]] && [[ -f runtime/trg5/run_trg_step.py ]]; then
    echo "TRIGGER $(date -u +%FT%TZ)"
    python3 "$scratch/runtime/trg5/run_trg_step.py" \
        --feb-file "$scratch/result$suffix" \
        --runtime-dir "$scratch/runtime" \
        --output-prefix "$scratch/result"
    test -s "result_trg.tar.gz"
fi

if [[ "$keep_feb" != "1" ]]; then
    echo "[INFO] Removing intermediate FEB binary (keep_feb=0)"
    rm -f "result$suffix"
    printf "FEB file discarded to save storage (keep_feb=0).\n" > result.bin
fi

printf '%s\n' "$job_key" > result.done
echo "FINISH $(date -u +%FT%TZ)"
ls -lh result.bin result_A_sums2 result_trg.tar.gz result.done
