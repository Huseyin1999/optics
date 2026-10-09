#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Package trusted Linux executables, dependencies, optics and trigger assets for Condor."""
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
    parser.add_argument("--hybrid", type=Path, required=True, help="Path to hybrid executable")
    parser.add_argument("--optics", type=Path, required=True, help="Path to TAIGA_optics_file executable")
    parser.add_argument("--assets", type=Path, required=True, help="Path to optics assets (parameters.txt, etc.)")
    parser.add_argument("--library", type=Path, action="append", default=[], help="Shared libraries (.so) to pack")
    parser.add_argument("--output", type=Path, default=Path("runtime.tar.gz"), help="Output archive path")

    # Trigger extensions (trg5)
    parser.add_argument("--trigger", type=Path, help="Path to trigger_iact executable")
    parser.add_argument("--cleaning", type=Path, help="Path to cleaning executable")
    parser.add_argument("--trg-scripts", type=Path, help="Path to trg5 Python scripts folder")
    parser.add_argument("--trg-assets", type=Path, help="Path to trg5 assets (Calibration/, trigger_config.json)")
    parser.add_argument("--amplitudes-file", type=Path, help="Path to XP1911 amplitudes distribution (probablies8.txt)")

    args = parser.parse_args()
    if args.output.name != "runtime.tar.gz":
        parser.error("Archive basename must be runtime.tar.gz")

    with tempfile.TemporaryDirectory(prefix="taiga_runtime_") as temporary:
        root = Path(temporary) / "runtime"
        for directory in ("bin", "lib", "assets", "trg5"):
            (root / directory).mkdir(parents=True)

        # 1. Base executables
        for source, name in ((args.hybrid, "hybrid"), (args.optics, "TAIGA_optics_file")):
            destination = root / "bin" / name
            shutil.copy2(source, destination)
            destination.chmod(0o755)

        # 2. Trigger executables
        if args.trigger:
            trg_dst = root / "bin" / "trigger_iact"
            shutil.copy2(args.trigger, trg_dst)
            trg_dst.chmod(0o755)
        if args.cleaning:
            clean_dst = root / "bin" / "cleaning"
            shutil.copy2(args.cleaning, clean_dst)
            clean_dst.chmod(0o755)

        # 3. Dynamic libraries
        for source in args.library:
            shutil.copy2(source, root / "lib" / source.name, follow_symlinks=True)

        # 4. Optics assets
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

        # 5. Trigger assets & calibrations
        trg_assets_dest = root / "assets" / "trg5"
        trg_assets_dest.mkdir(parents=True, exist_ok=True)

        if args.trg_assets and args.trg_assets.is_dir():
            is_calib_folder = (args.trg_assets.name == "Calibration")
            for source in args.trg_assets.rglob("*"):
                relative = source.relative_to(args.trg_assets)
                if any(part in ("obj", "build", "logs", ".git", "__pycache__", ".vscode") for part in relative.parts):
                    continue
                # Exclude build artifacts, sources, and binaries
                if source.is_file() and not source.name.endswith((".o", ".a", ".so", ".tar.gz", ".pyc", ".cpp", ".c", ".h", ".sh")):
                    if source.name in ("trigger_iact", "cleaning", "out.txt"):
                        continue
                    target = (trg_assets_dest / "Calibration" / relative) if is_calib_folder else (trg_assets_dest / relative)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(source, target)
                    print(f"[PACK] Included trigger asset: {target.relative_to(root)}")

            # Mirror Calibration directory to assets root and trg5 root for maximum compatibility
            calib_dir = trg_assets_dest / "Calibration"
            if calib_dir.is_dir():
                for alt_dest in (root / "assets" / "Calibration", root / "trg5" / "Calibration"):
                    alt_dest.parent.mkdir(parents=True, exist_ok=True)
                    if not alt_dest.exists():
                        try:
                            alt_dest.symlink_to(calib_dir)
                        except OSError:
                            shutil.copytree(calib_dir, alt_dest)

        # 6. Heavy amplitudes file (probablies8.txt)
        amp_file = args.amplitudes_file
        if not amp_file and args.trg_assets:
            cand = args.trg_assets / "probablies8.txt"
            if cand.is_file():
                amp_file = cand

        if amp_file and amp_file.is_file():
            shutil.copy2(amp_file, trg_assets_dest / "probablies8.txt")
            print(f"[PACK] Included amplitudes file: {amp_file} ({amp_file.stat().st_size} bytes)")
        else:
            print("[WARN] No amplitudes file (probablies8.txt) packaged. If required by trigger, specify --amplitudes-file.")

        # 7. Trigger scripts & runner
        here = Path(__file__).resolve().parent
        runner_candidate = here / "run_trg_step.py"
        if runner_candidate.is_file():
            shutil.copy2(runner_candidate, root / "trg5" / "run_trg_step.py")
            (root / "trg5" / "run_trg_step.py").chmod(0o755)
            print(f"[PACK] Included trigger runner: {runner_candidate.name} (zero-dependency pure Python)")

        if args.trg_scripts and args.trg_scripts.is_dir():
            for script_name in ("main_processing.py", "add_background_processing.py", "trigger_config.json"):
                cand = args.trg_scripts / script_name
                if cand.is_file():
                    shutil.copy2(cand, root / "trg5" / script_name)
                    print(f"[PACK] Included trg script/config: {script_name}")

        # Copy trigger_config.json to trg5 assets as default if present
        cfg_cand = trg_assets_dest / "trigger_config.json"
        if not cfg_cand.is_file() and (root / "trg5" / "trigger_config.json").is_file():
            shutil.copy2(root / "trg5" / "trigger_config.json", cfg_cand)

        # 8. Dependency check (ldd)
        env = dict(os.environ, LD_LIBRARY_PATH=str(root / "lib"))
        for binary in (root / "bin").iterdir():
            if binary.is_file() and os.access(binary, os.X_OK):
                check = subprocess.run(["ldd", str(binary)], env=env, text=True,
                                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                print(f"[LDD] {binary.name}:\n{check.stdout}")
                if "not found" in check.stdout:
                    print(f"[WARN] Some dependencies for {binary.name} not found:")
                    for line in check.stdout.splitlines():
                        if "not found" in line:
                            print(f"  {line}")
                    if "GLIBCXX" in check.stdout:
                        print("  -> Hint: Binary was compiled with a newer GCC. Recompile on this host: 'cd trg5_git && make'")
                    if "libMathMore" in check.stdout:
                        print("  -> Hint: Recompile with GSL without ROOT: 'cd trg5_git && make trigger_gsl' OR pass --library $(root-config --libdir)/libMathMore.so")

        manifest = {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                    for path in root.rglob("*") if path.is_file()}
        (root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        args.output.parent.mkdir(parents=True, exist_ok=True)

        # Publish archive atomically
        temporary_output = args.output.with_suffix(".gz.partial")
        with tarfile.open(temporary_output, "w:gz", dereference=True) as archive:
            archive.add(root, arcname="runtime")
        temporary_output.replace(args.output)
        print("Archive created successfully:", args.output.resolve(), "bytes:", args.output.stat().st_size)


if __name__ == "__main__":
    main()
