#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Prepare an isolated production campaign; never submit jobs implicitly."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import tarfile

HERE = Path(__file__).resolve().parent


def safe(path):
    path = path.resolve()
    if not re.fullmatch(r"[A-Za-z0-9_./:\\-]+", str(path)):
        raise ValueError("Use paths containing only letters, digits, _, -, /, .: " + str(path))
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_dir", type=Path, help="Directory containing CORSIKA input files")
    parser.add_argument("run_dir", type=Path, help="Directory for campaign run configuration and results")
    parser.add_argument("--runtime", type=Path, default=HERE / "runtime.tar.gz", help="Path to runtime.tar.gz")
    parser.add_argument("--pattern", default="*_iact.corsika", help="Filename pattern for input files")
    parser.add_argument("--radius", type=int, default=100, help="Separation radius in cm")
    parser.add_argument("--parameters", default="parameters.txt", help="Name of optics parameters file in runtime")
    parser.add_argument("--keep-feb", action="store_true", help="Retain FEB binary file in results (default: discarded to save space)")
    parser.add_argument("--trigger-config", type=Path, help="Optional custom trigger_config.json to embed in campaign")
    parser.add_argument("--limit", type=int, help="Limit number of jobs prepared")
    parser.add_argument("--force", action="store_true", help="Force rerun of completed inputs")
    args = parser.parse_args()

    source_dir, run_dir, runtime = map(safe, (args.input_dir, args.run_dir, args.runtime))
    if not source_dir.is_dir() or not runtime.is_file():
        parser.error("Input directory and runtime archive must exist")
    if args.radius <= 0 or (args.limit is not None and args.limit <= 0):
        parser.error("radius and limit must be positive")
    if not re.fullmatch(r"[A-Za-z0-9_.-]+", args.parameters):
        parser.error("parameters must be a simple filename")

    archive_hash = hashlib.sha256(runtime.read_bytes()).hexdigest()
    with tarfile.open(runtime, "r:gz") as archive:
        member = archive.extractfile("runtime/assets/" + args.parameters)
        if member is None:
            parser.error("Parameters not found in archive")
        parameters = {}
        for line in member.read().decode().splitlines():
            fields = line.split("//", 1)[0].split()
            if len(fields) >= 2:
                parameters[fields[0]] = fields[1]

    suffix = ("_s" if int(parameters.get("SPM", "0")) else "_f") + (
        "hb" if int(parameters.get("track_h", "0")) else "eb")

    sources = sorted(safe(path) for path in source_dir.glob(args.pattern) if path.is_file())
    if not sources:
        parser.error("No input files matched pattern: " + args.pattern)
    names = [path.name for path in sources]
    if len(names) != len(set(names)):
        parser.error("Duplicate input basenames; split these into separate campaigns")

    keep_feb_val = 1 if args.keep_feb else 0
    config = {
        "input_dir": str(source_dir),
        "runtime_sha256": archive_hash,
        "radius": args.radius,
        "parameters": args.parameters,
        "suffix": suffix,
        "keep_feb": keep_feb_val
    }

    config_file = run_dir / "campaign.json"
    if run_dir.exists() and not config_file.exists():
        parser.error("Use a new run directory, or an existing prepared campaign")
    if config_file.exists() and json.loads(config_file.read_text()) != config:
        parser.error("Campaign settings changed; use a new run directory")

    run_dir.mkdir(parents=True, exist_ok=True)
    for name in ("logs", "results"):
        (run_dir / name).mkdir(exist_ok=True)
    config_file.write_text(json.dumps(config, indent=2) + "\n")

    target_runtime = run_dir / "runtime.tar.gz"
    if not target_runtime.exists():
        shutil.copy2(runtime, target_runtime)
    elif hashlib.sha256(target_runtime.read_bytes()).hexdigest() != archive_hash:
        parser.error("Campaign runtime archive changed")

    shutil.copy2(HERE / "pipeline_one.sh", run_dir / "pipeline_one.sh")
    submit = (HERE / "pipeline.sub").read_text().replace(
        "radius = 100", "radius = " + str(args.radius)
    ).replace(
        "parameters_name = parameters.txt", "parameters_name = " + args.parameters
    ).replace(
        "keep_feb = 0", "keep_feb = " + str(keep_feb_val)
    )
    (run_dir / "pipeline.sub").write_text(submit)

    rows = []
    skipped = 0
    for source in sources:
        stat = source.stat()
        key = hashlib.sha256(json.dumps([config, str(source), stat.st_size,
                                         stat.st_mtime_ns], sort_keys=True).encode()).hexdigest()
        prefix = run_dir / "results" / source.name.removesuffix(".corsika")
        marker = Path(str(prefix) + ".done")
        products = [Path(str(prefix) + "_A_sums2"), Path(str(prefix) + "_trg.tar.gz")]
        if args.keep_feb:
            products.append(Path(str(prefix) + suffix))

        if not args.force and marker.is_file() and marker.read_text().strip() == key and all(
                product.is_file() and product.stat().st_size > 0 for product in products):
            skipped += 1
            continue
        rows.append(f"{source}\t{source.name}\t{prefix}\t{suffix}\t{key}\t{keep_feb_val}\n")

    if args.limit:
        rows = rows[:args.limit]
    (run_dir / "jobs.tsv").write_text("".join(rows))
    print(f"Prepared {len(rows)} jobs; skipped {skipped} completed inputs.")
    print(f"Campaign directory: {run_dir}")
    print("Run command: cd " + str(run_dir) + " && condor_submit pipeline.sub" if rows else "Nothing to submit.")


if __name__ == "__main__":
    main()
