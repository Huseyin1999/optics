#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Sequence


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Последовательно для каждого n запускает:\n"
            "  1) TAIGA_optics params_file n\n"
            "  2) main_processing.py json_cfg feb_path\n"
            "  3) опционально удаляет feb\n"
        ),
        formatter_class=argparse.RawTextHelpFormatter,
    )

    selector = parser.add_mutually_exclusive_group(required=True)
    selector.add_argument(
        "--range",
        nargs=2,
        type=int,
        metavar=("FIRST", "LAST"),
        help="Диапазон номеров включительно. Пример: --range 50002 50011",
    )
    selector.add_argument(
        "--numbers",
        nargs="+",
        type=int,
        help="Явный список номеров. Пример: --numbers 583 599 607",
    )
    selector.add_argument(
        "--source-dir",
        type=Path,
        help=(
            "Папка с файлами вида taigaHS{n}_i. "
            "Из имен будут автоматически извлечены номера n."
        ),
    )

    parser.add_argument(
        "--step",
        type=int,
        default=1,
        help="Шаг для --range. По умолчанию 1.",
    )
    parser.add_argument(
        "--recursive-source-search",
        action="store_true",
        help="Искать файлы в --source-dir рекурсивно.",
    )
    parser.add_argument(
        "--source-regex",
        default=r"^taigaHS(?P<n>\d+)_i(?:$|[._].*)",
        help=(
            "Регулярное выражение для извлечения номера из имени файла. "
            "Должна существовать группа (?P<n>...)."
        ),
    )

    parser.add_argument(
        "--taiga-optics",
        default="./TAIGA_optics",
        help="Путь к бинарнику TAIGA_optics.",
    )
    parser.add_argument(
        "--params-file",
        required=True,
        help="Путь к parameters*.txt для TAIGA_optics.",
    )
    parser.add_argument(
        "--feb-template",
        required=True,
        help=(
            "Шаблон пути к feb-файлу после генерации.\n"
            'Например: "/k38/.../bpe{n}_33_da0.0_md5/taiga{n}_feb"'
        ),
    )

    parser.add_argument(
        "--python-bin",
        default="python3",
        help="Интерпретатор Python для запуска processing-скрипта.",
    )
    parser.add_argument(
        "--processing-script",
        default="main_processing.py",
        help="Путь к main_processing.py.",
    )
    parser.add_argument(
        "--json-configs",
        nargs="+",
        required=True,
        help="Один или несколько JSON-конфигов для main_processing.py.",
    )

    parser.add_argument(
        "--delete-feb",
        action="store_true",
        help="Удалять feb после успешной обработки всеми JSON-конфигами.",
    )
    parser.add_argument(
        "--continue-on-error",
        action="store_true",
        help="Не останавливать весь пайплайн при ошибке на одном n.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Только показать, что будет запущено, без реального запуска.",
    )
    parser.add_argument(
        "--workdir",
        default=None,
        help="Рабочая директория, из которой запускать команды.",
    )

    return parser.parse_args()


def build_range(first: int, last: int, step: int) -> List[int]:
    if step == 0:
        raise ValueError("step не может быть 0")

    if first < last and step < 0:
        raise ValueError("Для возрастающего диапазона нужен положительный step")
    if first > last and step > 0:
        raise ValueError("Для убывающего диапазона нужен отрицательный step")

    if first == last:
        return [first]

    stop = last + (1 if step > 0 else -1)
    numbers = list(range(first, stop, step))
    if not numbers:
        raise ValueError("Диапазон пустой")
    return numbers


def discover_numbers(source_dir: Path, pattern: str, recursive: bool) -> List[int]:
    if not source_dir.exists():
        raise FileNotFoundError("Папка не найдена: {}".format(source_dir))
    if not source_dir.is_dir():
        raise NotADirectoryError("Это не папка: {}".format(source_dir))

    regex = re.compile(pattern)
    iterator = source_dir.rglob("*") if recursive else source_dir.iterdir()

    numbers = set()
    for path in iterator:
        match = regex.match(path.name)
        if match:
            numbers.add(int(match.group("n")))

    if not numbers:
        raise ValueError(
            "В папке {} не найдено файлов по шаблону {!r}".format(source_dir, pattern)
        )

    return sorted(numbers)


def resolve_numbers(args: argparse.Namespace) -> List[int]:
    if args.numbers:
        return sorted(set(args.numbers))

    if args.range:
        first, last = args.range
        return build_range(first, last, args.step)

    return discover_numbers(
        source_dir=args.source_dir,
        pattern=args.source_regex,
        recursive=args.recursive_source_search,
    )


def format_feb_path(template: str, n: int) -> Path:
    try:
        return Path(template.format(n=n))
    except KeyError as exc:
        raise ValueError(
            "Не удалось подставить n в feb-template={!r}. Ошибка: {}".format(
                template, exc
            )
        )


def remove_path(path: Path, dry_run: bool = False) -> None:
    if not path.exists():
        print("[WARN] Для удаления путь не найден: {}".format(path))
        return

    if dry_run:
        print("[DRY-RUN] Удаление: {}".format(path))
        return

    if path.is_file() or path.is_symlink():
        path.unlink()
    elif path.is_dir():
        shutil.rmtree(str(path))
    else:
        raise RuntimeError("Неизвестный тип пути, не могу удалить: {}".format(path))


def run_cmd(cmd: Sequence[str], cwd: Optional[str] = None, dry_run: bool = False) -> None:
    printable = " ".join(shlex.quote(str(x)) for x in cmd)
    print("$ {}".format(printable))
    if dry_run:
        return
    subprocess.run([str(x) for x in cmd], check=True, cwd=cwd)


def process_one_number(n: int, args: argparse.Namespace) -> None:
    print("\n========== n = {} ==========".format(n))

    run_cmd(
        [args.taiga_optics, args.params-file if False else args.params_file, str(n)],
        cwd=args.workdir,
        dry_run=args.dry_run,
    )

    feb_path = format_feb_path(args.feb_template, n)
    print("[INFO] Ожидаемый feb: {}".format(feb_path))

    if (not args.dry_run) and (not feb_path.exists()):
        raise FileNotFoundError(
            "После TAIGA_optics feb не найден: {}\n"
            "Проверьте --feb-template и параметры генерации.".format(feb_path)
        )

    for json_cfg in args.json_configs:
        run_cmd(
            [args.python_bin, args.processing_script, json_cfg, str(feb_path)],
            cwd=args.workdir,
            dry_run=args.dry_run,
        )

    if args.delete_feb:
        remove_path(feb_path, dry_run=args.dry_run)


def main() -> int:
    args = parse_args()

    try:
        numbers = resolve_numbers(args)
    except Exception as exc:
        print("[FATAL] Не удалось сформировать список номеров: {}".format(exc), file=sys.stderr)
        return 2

    print("[INFO] Номера для обработки: {}".format(numbers))

    failed = []
    for n in numbers:
        try:
            process_one_number(n, args)
        except Exception as exc:
            print("[ERROR] n={}: {}".format(n, exc), file=sys.stderr)
            failed.append(n)
            if not args.continue_on_error:
                break

    print("\n========== ИТОГ ==========")
    ok = len(numbers) - len(failed)
    print("Успешно: {}".format(ok))
    print("Ошибки:  {}".format(len(failed)))
    if failed:
        print("Не обработаны/упали: {}".format(failed))
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())