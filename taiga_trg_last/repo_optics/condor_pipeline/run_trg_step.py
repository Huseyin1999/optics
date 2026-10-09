#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
run_trg_step.py - Standalone trigger, NSB background and Hillas cleaning runner
for TAIGA-IACT in HTCondor worker scratch environment.

PURE PYTHON STANDARD LIBRARY ONLY:
Zero dependencies on numpy, pandas, or scipy!
Runs out-of-the-box on ANY minimal Linux worker node (EL9, Debian, JINR, etc.).

Executes the full trg5 chain:
  1) Generates camera pixel & neighbor mapping (pure math/csv)
  2) trigger_iact: reads FEB binary, simulates PMT response & trigger coincidence
  3) apply_nsb_background: simulates Night Sky Background (random.gauss)
  4) cleaning: dual-threshold tail-cut cleaning and Hillas parameters calculation
  5) Packages results into {output_prefix}_trg.tar.gz and extracts Hillas CSVs
"""

import argparse
import csv
import json
import math
import os
from pathlib import Path
import random
import re
import shutil
import subprocess
import sys
import tarfile


def rotate(point, angle_rad, origin=(0.0, 0.0)):
    ox, oy = origin
    px, py = point
    cos_a = math.cos(angle_rad)
    sin_a = math.sin(angle_rad)
    qx = ox + cos_a * (px - ox) - sin_a * (py - oy)
    qy = oy + sin_a * (px - ox) + cos_a * (py - oy)
    return qx, qy


def resolve_asset_path(raw_path, search_dirs):
    """
    Resolve asset path robustly:
    1. If already exists (absolute or relative), return it.
    2. Check relative to each search directory.
    3. Check by filename inside search directories and their Calibration subdirectories.
    4. Fallback: recursive rglob search by filename in all search directories.
    """
    if not raw_path:
        return ""
    p = Path(raw_path)
    if p.exists():
        return str(p.resolve())

    for base in search_dirs:
        candidate = base / p
        if candidate.exists():
            return str(candidate.resolve())
        candidate_calib = base / "Calibration" / p.name
        if candidate_calib.exists():
            return str(candidate_calib.resolve())
        candidate_name = base / p.name
        if candidate_name.exists():
            return str(candidate_name.resolve())

    # Fallback: search recursively by file name in all search directories
    for base in search_dirs:
        if base.exists() and base.is_dir():
            for match in base.rglob(p.name):
                if match.is_file():
                    return str(match.resolve())

    all_found = []
    for base in search_dirs:
        if base.exists() and base.is_dir():
            all_found.extend([str(f.relative_to(base)) for f in base.rglob("*") if f.is_file()])

    raise FileNotFoundError(
        f"Cannot resolve asset path: '{raw_path}' (target filename: '{p.name}')\n"
        f"Searched in: {[str(d) for d in search_dirs]}\n"
        f"Available files in search dirs (first 20): {all_found[:20]}"
    )


def get_config(iact_numb, path_coord, path_sense, use_sense, save_path):
    """
    Generates IACT0{iact_numb}_cam_corsika_config.csv using pure standard library.
    No pandas or numpy required!
    """
    # 1. Read coordinates: [cluster, x_cm, y_cm, channel]
    # In exp_coord file columns: col 0 = cluster, col 3 = x_cm, col 4 = y_cm, col 7 = channel
    exp_coord = []
    with open(path_coord, 'r', encoding='utf-8') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 8:
                clust = int(parts[0])
                x_cm = float(parts[3])
                y_cm = float(parts[4])
                chan = int(parts[7])
                exp_coord.append((clust, x_cm, y_cm, chan))

    # 2. Read sensitivities: [Cluster, Channel, Rel_sens]
    # Header usually: Cluster Channel ... Rel_sens
    sense_map = {}
    max_sense = 1.0
    with open(path_sense, 'r', encoding='utf-8') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 4 and parts[0].isdigit():
                try:
                    c_clust = int(parts[0])
                    c_chan = int(parts[1])
                    rel_s = float(parts[3])
                    sense_map[(c_clust, c_chan)] = rel_s
                    if rel_s > max_sense:
                        max_sense = rel_s
                except ValueError:
                    continue

    s_pmt = 30.0
    rot_angle_rad = 37.5 * math.pi / 180.0
    cm_to_deg = 0.1206
    trig_circle_deg = 3.7
    pixel_cm_width = 3.0
    columns_table = [
        'pixel_number', 'cluster', 'channel', 'row', 'col',
        'x_cam', 'y_cam', 'triger', 'sense', 'n_cluster_neighbours',
        '1', '2', '3', '4', '5', '6'
    ]

    rows = []
    n_pix = len(exp_coord)

    for i in range(n_pix):
        clust, x_val, y_val, chan = exp_coord[i]
        coord_corsika = rotate((x_val, y_val), rot_angle_rad)
        nr = 2.0 * coord_corsika[1] * 10.0 / (s_pmt * math.sqrt(3.0))
        nc = ((2.0 * coord_corsika[0] * 10.0 / s_pmt) + (round(nr) % 2)) / 2.0

        pmt_x = ((s_pmt / 2.0) * (2.0 * round(nc) - (round(nr) % 2))) / 10.0
        pmt_y = round(nr) * s_pmt * math.sqrt(3.0) / 20.0

        distance0 = cm_to_deg * math.sqrt(x_val * x_val + y_val * y_val)
        trig = 1 if distance0 < trig_circle_deg else 0

        neigh_arr = []
        for j in range(n_pix):
            _, jx, jy, _ = exp_coord[j]
            dist = math.sqrt((jx - x_val) ** 2 + (jy - y_val) ** 2)
            if (pixel_cm_width - 0.5) < dist < (pixel_cm_width + 0.5):
                neigh_arr.append(j)

        n_neigh = len(neigh_arr)
        while len(neigh_arr) < 6:
            neigh_arr.append(-1)

        sense = 1.0
        if use_sense == 1:
            raw_s = sense_map.get((clust, chan * 2), None)
            if raw_s is not None and max_sense > 0:
                sense = raw_s / max_sense

        rows.append([
            i, clust, chan, int(round(nr)), int(round(nc)),
            f"{pmt_x:.4f}", f"{pmt_y:.4f}", trig, f"{sense:.4f}", n_neigh,
            *neigh_arr
        ])

    config_path = os.path.join(save_path, f"IACT0{iact_numb}_cam_corsika_config.csv")
    with open(config_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(columns_table)
        writer.writerows(rows)

    last_clust = exp_coord[-1][0] if exp_coord else 0
    return n_pix, last_clust, config_path, float(max_sense)


def get_slow_pulse_const(path):
    with open(path, 'r', encoding='utf-8') as f:
        rows = []
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                try:
                    rows.append((float(parts[0]), float(parts[1])))
                except ValueError:
                    continue
    if not rows:
        raise ValueError(f"Empty pulse file: {path}")
    tim0 = rows[0][0]
    max_amp = max(r[1] for r in rows)
    return tim0, max_amp, len(rows)


def apply_nsb_background(in_file, out_file, config_cam_path, mean, std):
    """
    Adds Night Sky Background (NSB) Gaussian noise to triggered amplitudes.
    Uses pure Python random.gauss. No numpy required!
    """
    with open(in_file, 'r', encoding='utf-8') as fin, open(out_file, 'w', encoding='utf-8') as fout:
        for line in fin:
            parts = line.strip().split()
            if not parts:
                continue
            if len(parts) > 5:
                # Event header
                fout.write(line)
            elif len(parts) == 5:
                # Pixel hit: cluster channel x_cam y_cam amplitude
                clust = parts[0]
                chan = parts[1]
                x_cam = parts[2]
                y_cam = parts[3]
                amp = float(parts[4])
                noise = random.gauss(mean, std)
                total_amp = amp + noise
                fout.write(f"{clust}\t{chan}\t{x_cam}\t{y_cam}\t{total_amp:.3f}\n")


def flatten_list(nested):
    res = []
    if isinstance(nested, (list, tuple)):
        for item in nested:
            res.extend(flatten_list(item))
    else:
        res.append(nested)
    return res


def run_command(cmd, log_prefix=""):
    print(f"[CMD] {' '.join(str(c) for c in cmd)}")
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        print(f"[ERROR] Command failed with exit code {res.returncode}:\nSTDERR:\n{res.stderr}\nSTDOUT:\n{res.stdout}", file=sys.stderr)
        raise RuntimeError(f"Command failed: {' '.join(str(c) for c in cmd)}")
    return res.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--feb-file", type=Path, required=True, help="Input .feb binary file from TAIGA_optics")
    parser.add_argument("--runtime-dir", type=Path, default=Path("runtime"), help="Runtime root folder")
    parser.add_argument("--output-prefix", type=Path, default=Path("result"), help="Prefix for output products")
    parser.add_argument("--config", type=Path, help="Custom trigger_config.json path")
    parser.add_argument("--clean-edge1", type=float, help="Override cleaning threshold 1 (default from config, e.g. 14.0)")
    parser.add_argument("--clean-edge2", type=float, help="Override cleaning threshold 2 (default from config, e.g. 7.0)")
    parser.add_argument("--gam", type=int, help="Override gam parameter (1: real direction, 0: scattered)")
    parser.add_argument("--keep-intermediate", action="store_true", help="Keep raw _c.txt and _cb.txt files")
    args = parser.parse_args()

    feb_file = args.feb_file.resolve()
    if not feb_file.is_file():
        sys.exit(f"[FATAL] FEB file not found: {feb_file}")

    runtime_dir = args.runtime_dir.resolve()
    scratch_dir = Path.cwd().resolve()
    work_out = scratch_dir / "trg_work"
    work_out.mkdir(parents=True, exist_ok=True)

    search_dirs = [
        scratch_dir,
        runtime_dir / "assets" / "trg5",
        runtime_dir / "assets" / "trg5" / "Calibration",
        runtime_dir / "assets",
        runtime_dir / "assets" / "Calibration",
        runtime_dir / "trg5",
        runtime_dir / "trg5" / "Calibration",
        runtime_dir,
    ]

    # Find config
    config_file = args.config if args.config else None
    if not config_file:
        for d in search_dirs:
            cand = d / "trigger_config.json"
            if cand.is_file():
                config_file = cand
                break
    if not config_file or not config_file.is_file():
        sys.exit(f"[FATAL] trigger_config.json not found in {[str(d) for d in search_dirs]}")

    with open(config_file, 'r', encoding='utf-8') as f:
        cfg = json.load(f)

    # Binaries
    trigger_bin = runtime_dir / "bin" / "trigger_iact"
    if not trigger_bin.is_file():
        trigger_bin = Path("trigger_iact").resolve()
    cleaning_bin = runtime_dir / "bin" / "cleaning"
    if not cleaning_bin.is_file():
        cleaning_bin = Path("cleaning").resolve()

    if not trigger_bin.is_file() or not cleaning_bin.is_file():
        sys.exit(f"[FATAL] Required binaries missing: trigger_iact={trigger_bin}, cleaning={cleaning_bin}")

    # Overrides
    clean_edge1 = args.clean_edge1 if args.clean_edge1 is not None else float(cfg.get("clean_edge1", 14.0))
    clean_edge2 = args.clean_edge2 if args.clean_edge2 is not None else float(cfg.get("clean_edge2", 7.0))
    gam = args.gam if args.gam is not None else int(cfg.get("gam", 1))

    # Resolve calibration assets
    iacts = cfg.get("IACTs", [1, 2, 3, 4, 5])
    pulse_files = [resolve_asset_path(p, search_dirs) for p in cfg["pulse_files"]]
    coord_files = [resolve_asset_path(p, search_dirs) for p in cfg["coord_files"]]
    factors_path = [resolve_asset_path(p, search_dirs) for p in cfg["factors_path"]]
    amplitudes_file = resolve_asset_path(cfg.get("amplitudes_file", "probablies8.txt"), search_dirs)

    use_sense = int(cfg.get("use_sense", 1))
    trigger_type = cfg.get("trigger_type", [1] * len(iacts))
    integrate_window = cfg.get("integrate_window", [80] * len(iacts))
    t_grid = cfg.get("t_grid", 1)
    t_sign = cfg.get("t_sign", 20)
    trig_amp = float(cfg.get("trig_amp", 10.0))
    t_hold = cfg.get("t_hold", 160)
    trig_window = cfg.get("trig_window", 15)
    t_after_before = cfg.get("t_after_before", 100)
    sense_cam = cfg.get("sense_cam", [1.0] * len(iacts))
    hidden_mirror = cfg.get("hidden_mirror", [[] for _ in iacts])
    background = int(cfg.get("background", 0))
    bg_params = cfg.get("background_params", {})
    mean_ph = 3
    mean_phe_time = 35
    if "2" in bg_params:
        mean_ph = bg_params["2"].get("mean_ph", 3)
        mean_phe_time = bg_params["2"].get("mean_phe_time", 35)

    source_scatter_radius = float(cfg.get("source_scatter_radius", 5.0))
    use_sigma_for_cleaning = int(cfg.get("use_sigma_for_cleaning", 0))
    use_brightest_island = int(cfg.get("use_brightest_island", 1))

    print(f"[INFO] Running trg5 on {feb_file.name} for IACTs {iacts} (Pure Python mode)...")

    generated_hillas_files = []
    generated_clean_files = []
    generated_all_files = []

    for idx, iact in enumerate(iacts):
        print(f"\n--- Processing IACT0{iact} ---")
        n_pix, n_clusters, config_cam_path, max_sense = get_config(
            iact, coord_files[iact - 1], factors_path[iact - 1], use_sense, str(work_out)
        )
        tim0, max_amp, len_pulse = get_slow_pulse_const(pulse_files[iact - 1])

        out_c_file = work_out / f"iact0{iact}_c.txt"
        out_cb_file = work_out / f"iact0{iact}_cb0.txt"
        out_clean_file = work_out / f"iact0{iact}_clean.txt"
        out_hillas_file = work_out / f"iact0{iact}_hillas.csv"

        flat_hidden = flatten_list(hidden_mirror[iact - 1])

        # Stage 1: trigger_iact
        trigger_cmd = [
            str(trigger_bin),
            str(iact),
            str(n_pix),
            str(n_clusters),
            str(trigger_type[iact - 1]),
            str(t_grid),
            str(t_sign),
            str(trig_amp),
            str(t_hold),
            str(integrate_window[iact - 1]),
            str(trig_window),
            str(t_after_before),
            str(len_pulse),
            str(tim0),
            str(max_amp),
            str(background),
            str(mean_ph),
            str(mean_phe_time),
            str(config_cam_path),
            str(amplitudes_file),
            str(out_c_file),
            str(feb_file),
            str(pulse_files[iact - 1]),
            str(sense_cam[iact - 1]),
            str(int(len(flat_hidden) / 2)),
            *[str(x) for x in flat_hidden]
        ]
        run_command(trigger_cmd, f"trigger_iact_iact0{iact}")

        if not out_c_file.is_file() or out_c_file.stat().st_size == 0:
            print(f"[WARN] No triggered events for IACT0{iact}")
            out_hillas_file.write_text("event_id,size,length,width,dist,alpha,azwidth,miss\n")
            out_clean_file.write_text("")
            generated_hillas_files.append(out_hillas_file)
            generated_clean_files.append(out_clean_file)
            continue

        generated_all_files.append(out_c_file)

        # Stage 2: NSB background
        mean_bg = 0.0
        std_bg = 3.9
        try:
            dist_bg = bg_params.get("0", {}).get("distribution background", {})
            bg_tel = dist_bg.get(str(iact), dist_bg.get(iact, {}))
            mean_bg = float(bg_tel.get("mean", 0.0))
            std_bg = float(bg_tel.get("std", 3.9))
        except Exception:
            pass

        apply_nsb_background(str(out_c_file), str(out_cb_file), config_cam_path, mean_bg, std_bg)
        generated_all_files.append(out_cb_file)

        # Stage 3: Cleaning & Hillas parameters
        sigma_file = factors_path[iact - 1]
        cleaning_cmd = [
            str(cleaning_bin),
            str(iact),
            str(gam),
            str(n_pix),
            str(clean_edge1),
            str(clean_edge2),
            str(config_cam_path),
            str(out_cb_file),
            str(out_clean_file),
            str(out_hillas_file),
            str(source_scatter_radius),
            str(factors_path[iact - 1]),
            str(sigma_file),
            str(use_sigma_for_cleaning),
            str(use_brightest_island)
        ]
        run_command(cleaning_cmd, f"cleaning_iact0{iact}")

        if out_hillas_file.is_file():
            generated_hillas_files.append(out_hillas_file)
            generated_all_files.append(out_hillas_file)
        if out_clean_file.is_file():
            generated_clean_files.append(out_clean_file)
            generated_all_files.append(out_clean_file)

    # Copy individual Hillas CSVs to final output location with prefix
    out_prefix_str = str(args.output_prefix)
    for hf in generated_hillas_files:
        tel_match = re.search(r"iact0\d", hf.name)
        tel_tag = f"_{tel_match.group(0)}" if tel_match else ""
        dst = Path(f"{out_prefix_str}{tel_tag}_hillas.csv")
        shutil.copy2(hf, dst)
        print(f"[OUT] Hillas CSV: {dst} ({dst.stat().st_size} bytes)")

    # Bundle all trigger products into {output_prefix}_trg.tar.gz
    trg_tar_path = Path(f"{out_prefix_str}_trg.tar.gz")
    with tarfile.open(trg_tar_path, "w:gz") as tar:
        for f in work_out.iterdir():
            if f.is_file():
                tar.add(f, arcname=f.name)
    print(f"[OUT] Trigger bundle: {trg_tar_path} ({trg_tar_path.stat().st_size} bytes)")

    # Cleanup intermediate scratch if requested
    if not args.keep_intermediate:
        shutil.rmtree(work_out, ignore_errors=True)

    print("[SUCCESS] Full trg5 pipeline stage completed successfully.")


if __name__ == "__main__":
    main()
