#!/usr/bin/env python3
"""
Параллельный запуск main_processing.py
для набора путей и нескольких JSON-конфигов.

Теперь поддерживается несколько "пакетов" путей:
каждый пакет = (PATH_TEMPLATE + диапазон номеров first,last).
Пакеты обрабатываются последовательно.
"""

import subprocess
import multiprocessing as mp
from dataclasses import dataclass
from typing import Iterable, List, Optional, Sequence, Tuple


# ──────────────────── КОНФИГ ──────────────────────

# Степень параллелизма (как и было у вас, можно оставить 1)
PROCESSES = 3

# Можно ускорить выдачу задач воркерам (необязательно)
CHUNKSIZE: Optional[int] = None  # например 50; или None = по умолчанию у map()

# Набор конфиг-файлов, которые надо прогнать последовательно
JSON_CONFIGS = [
    "./configs/trigger_config_IACT01.json",
    "./configs/trigger_config_IACT02.json",
]

@dataclass(frozen=True)
class PathJob:
    """
    Один "пакет" путей:
      - template: строка с {n} (можно и {n:05d})
      - first,last: границы диапазона ВКЛЮЧИТЕЛЬНО
      - step: шаг (по умолчанию 1)
      - manual: если задан непустой список, он приоритетнее диапазона
    """
    name: str
    template: str
    first: int
    last: int
    step: int = 1
    manual: Optional[Sequence[int]] = None


# Список шаблонов/диапазонов для ПОСЛЕДОВАТЕЛЬНОЙ обработки
PATH_JOBS: List[PathJob] = [
    # PathJob(
    #     name="50001_Protons",
    #     template="/k37-1/data/rnf22/feb/protons_40-400TeV/bpe{n}_33m_da5.0_md5_old_cone/taiga{n}_feb",
    #     first=50001,
    #     last=52000,  # включительно
    # ),
        PathJob(
        name="O_Fe_40-400",
        template="/k38/taiga_pool/CORSIKA_TAIGA/mc2/20-22/bpe{n}_33m_da5.0_md5_old_cone/taiga{n}_feb",
        first=910,
        last=1005,  # включительно
    ),
    #     PathJob(
    #     name="1454_Muons",
    #     template="/k38/taiga_pool/CORSIKA_TAIGA/mc2/20-22/bpe{n}_33m_da5.0_md5_old_cone/taiga{n}_feb",
    #     first=1454,
    #     last=1469,  # включительно
    # ),
    # PathJob(
    #     name="1710_He",
    #     template="/k38/taiga_pool/CORSIKA_TAIGA/mc/all1710/bpe{n}_33_da0.0_md5/taiga{n}_feb",
    #     first=1710,
    #     last=1725, 
    # ),
    # PathJob(
    #     name="1742_Fe",
    #     template="/k38/taiga_pool/CORSIKA_TAIGA/mc/all1742/bpe{n}_33_da0.0_md5/taiga{n}_feb",
    #     first=1742,
    #     last=1773, 
    # ),
    # PathJob(
    #     name="1054_H",
    #     template="/k38/taiga_pool/CORSIKA_TAIGA/mc/all1054/bpe{n}_31_da0.0_md5_a2021/taiga{n}_feb",
    #     first=1054,
    #     last=1101, 
    # ),
]

# ──────────────────────────────────────────────────


def launch_one(task: Tuple[str, str]) -> None:
    """Запускает main_processing.py для пары (json_cfg, path)."""
    json_cfg, path = task
    try:
        subprocess.run(
            ["python3", "main_processing.py", json_cfg, path],
            check=True,
        )
    except subprocess.CalledProcessError as exc:
        print(f"[ERROR] {json_cfg}: код {exc.returncode} для {path}")
        # raise  # если хотите полностью остановиться — раскомментируйте


def iter_numbers(job: PathJob) -> Iterable[int]:
    """Возвращает номера для конкретного шаблона (manual имеет приоритет)."""
    if job.manual:
        return job.manual
    if job.step == 0:
        raise ValueError(f"step не может быть 0 (job={job.name})")
    if job.first <= job.last and job.step < 0:
        raise ValueError(f"step отрицательный, но first<=last (job={job.name})")
    if job.first >= job.last and job.step > 0:
        # всё равно допустимо, но range будет пустой; лучше подсказать явно
        raise ValueError(f"first > last при положительном step (job={job.name})")
    # last включительно → range(stop=last+1) при положительном step
    return range(job.first, job.last + (1 if job.step > 0 else -1), job.step)


def build_paths(job: PathJob) -> List[str]:
    """Формирует список путей для данного job."""
    paths: List[str] = []
    for n in iter_numbers(job):
        # поддерживает и {n}, и {n:05d}, и любые format-спеки
        paths.append(job.template.format(n=n))
    return paths


def map_tasks(pool: mp.Pool, tasks: List[Tuple[str, str]]) -> None:
    """Обертка, чтобы аккуратно применить chunksize (если задан)."""
    if not tasks:
        return
    if CHUNKSIZE is None:
        pool.map(launch_one, tasks)
    else:
        pool.map(launch_one, tasks, chunksize=CHUNKSIZE)


if __name__ == "__main__":
    with mp.Pool(processes=PROCESSES) as pool:
        # Последовательная обработка шаблонов
        for job in PATH_JOBS:
            print(f"\n=== Пакет путей: {job.name} ===")
            paths = build_paths(job)
            print(f"Путей: {len(paths)}  |  диапазон: {job.first},{job.last}  |  step={job.step}")

            # Последовательная обработка конфигов (как и было)
            for json_cfg in JSON_CONFIGS:
                print(f"--- Конфиг: {json_cfg} ---")
                tasks = [(json_cfg, path) for path in paths]
                map_tasks(pool, tasks)
