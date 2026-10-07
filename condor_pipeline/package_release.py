#!/usr/bin/env python3
"""Create a portable distribution with scripts, sources, docs and runtime."""
from pathlib import Path
import tarfile

root = Path(__file__).resolve().parent
runtime = root / "runtime.tar.gz"
if not runtime.is_file():
    raise SystemExit("Build runtime.tar.gz with pack_runtime.py first")
output = root / "condor_pipeline_release.tar.gz"
with tarfile.open(output, "w:gz") as archive:
    for path in sorted(root.iterdir()):
        if path.is_file() and (path.name in ("runtime.tar.gz", "Makefile") or
                               path.suffix in (".py", ".sh", ".sub", ".md", ".cpp")):
            archive.add(path, arcname="condor_pipeline/" + path.name)
print(output)
