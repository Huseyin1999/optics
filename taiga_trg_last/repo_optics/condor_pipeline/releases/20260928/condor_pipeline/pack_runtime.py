#!/usr/bin/env python3
"""Package trusted Linux executables, dependencies and optical assets for Condor."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--hybrid", type=Path, required=True)
    parser.add_argument("--optics", type=Path, required=True)
    parser.add_argument("--assets", type=Path, required=True)
    parser.add_argument("--library", type=Path, action="append", required=True)
    parser.add_argument("--output", type=Path, default=Path("runtime.tar.gz"))
    args = parser.parse_args()
    if args.output.name != "runtime.tar.gz":
        parser.error("Archive basename must be runtime.tar.gz")
    with tempfile.TemporaryDirectory(prefix="taiga_runtime_") as temporary:
        root = Path(temporary) / "runtime"
        for directory in ("bin", "lib", "assets"):
            (root / directory).mkdir(parents=True)
        for source, name in ((args.hybrid, "hybrid"), (args.optics, "TAIGA_optics_file")):
            destination = root / "bin" / name
            shutil.copy2(source, destination)
            destination.chmod(0o755)
        for source in args.library:
            shutil.copy2(source, root / "lib" / source.name, follow_symlinks=True)
        # Preserve relative paths; exclude build products and logs.
        for source in args.assets.rglob("*"):
            relative = source.relative_to(args.assets)
            if any(part in ("obj", "build", "logs", ".git") for part in relative.parts):
                continue
            if source.is_file() and source.suffix in (".txt", ".dat"):
                target = root / "assets" / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
        if not (root / "assets" / "parameters.txt").is_file():
            parser.error("Assets must contain parameters.txt")
        env = dict(os.environ, LD_LIBRARY_PATH=str(root / "lib"))
        for binary in (root / "bin").iterdir():
            check = subprocess.run(["ldd", str(binary)], env=env, text=True,
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            print(check.stdout)
            if check.returncode or "not found" in check.stdout:
                raise RuntimeError("Unresolved dependencies: " + binary.name)
        manifest = {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                    for path in root.rglob("*") if path.is_file()}
        (root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        args.output.parent.mkdir(parents=True, exist_ok=True)
        # Publish only a fully written archive.
        temporary_output = args.output.with_suffix(".gz.partial")
        with tarfile.open(temporary_output, "w:gz", dereference=True) as archive:
            archive.add(root, arcname="runtime")
        temporary_output.replace(args.output)
        print("Archive:", args.output.resolve(), "bytes:", args.output.stat().st_size)


if __name__ == "__main__":
    main()
