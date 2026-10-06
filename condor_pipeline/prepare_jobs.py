#!/usr/bin/env python3
"""Build HTCondor item-data lists: one input file per job."""

from __future__ import annotations

import argparse
from pathlib import Path


OPTICS_SUFFIXES = ("_feb", "_seb", "_fhb", "_shb")


def safe_path(path: Path) -> Path:
    resolved = path.resolve()
    if any(character.isspace() for character in str(resolved)):
        raise SystemExit(f"HTCondor item paths must not contain whitespace: {resolved}")
    return resolved


def converted_prefix(output_dir: Path, source: Path) -> Path:
    name = source.name[:-len(".corsika")] if source.name.endswith(".corsika") else source.stem
    return output_dir / f"{name}_split"


def optics_prefix(output_dir: Path, source: Path) -> Path:
    name = source.name[:-2] if source.name.endswith("_i") else source.stem
    return output_dir / name


def conversion_complete(prefix: Path, source: Path) -> bool:
    products = (Path(str(prefix) + "_t"), Path(str(prefix) + "_i"))
    return all(
        product.is_file()
        and product.stat().st_size > 0
        and product.stat().st_mtime >= source.stat().st_mtime
        for product in products
    )


def simulation_complete(prefix: Path, source: Path, parameters: Path) -> bool:
    marker = Path(str(prefix) + ".done")
    newest_input = max(source.stat().st_mtime, parameters.stat().st_mtime)
    products = [Path(str(prefix) + suffix) for suffix in OPTICS_SUFFIXES]
    return (
        marker.is_file()
        and marker.stat().st_mtime >= newest_input
        and any(product.is_file() and product.stat().st_size > 0 for product in products)
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="stage", required=True)

    convert = subparsers.add_parser("convert", help="prepare io_taiga/hybrid jobs")
    convert.add_argument("input_dir", type=Path)
    convert.add_argument("output_dir", type=Path)
    convert.add_argument("--pattern", default="*_iact.corsika")
    convert.add_argument("--jobs-file", type=Path, default=Path("convert_jobs.tsv"))
    convert.add_argument("--force", action="store_true")

    simulate = subparsers.add_parser("simulate", help="prepare TAIGA_optics jobs")
    simulate.add_argument("input_dir", type=Path)
    simulate.add_argument("output_dir", type=Path)
    simulate.add_argument("--parameters", type=Path, required=True)
    simulate.add_argument("--pattern", default="*_i")
    simulate.add_argument("--jobs-file", type=Path, default=Path("simulate_jobs.tsv"))
    simulate.add_argument("--force", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    input_dir = safe_path(args.input_dir)
    output_dir = safe_path(args.output_dir)
    jobs_file = safe_path(args.jobs_file)
    if not input_dir.is_dir():
        raise SystemExit(f"Input directory does not exist: {input_dir}")
    output_dir.mkdir(parents=True, exist_ok=True)
    jobs_file.parent.mkdir(parents=True, exist_ok=True)

    parameters = safe_path(args.parameters) if args.stage == "simulate" else None
    if parameters is not None and not parameters.is_file():
        raise SystemExit(f"Parameters file does not exist: {parameters}")

    rows: list[tuple[Path, Path]] = []
    skipped = 0
    sources = sorted(path for path in input_dir.glob(args.pattern) if path.is_file())
    for source0 in sources:
        source = safe_path(source0)
        if args.stage == "convert":
            prefix = converted_prefix(output_dir, source)
            complete = conversion_complete(prefix, source)
        else:
            prefix = optics_prefix(output_dir, source)
            complete = simulation_complete(prefix, source, parameters)
        if complete and not args.force:
            skipped += 1
        else:
            rows.append((source, prefix))

    content = "".join(f"{source}\t{prefix}\n" for source, prefix in rows)
    jobs_file.write_text(content, encoding="utf-8")
    print(f"Stage: {args.stage}")
    print(f"Found: {len(sources)}; queued: {len(rows)}; skipped complete: {skipped}")
    print(f"Job list: {jobs_file}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
