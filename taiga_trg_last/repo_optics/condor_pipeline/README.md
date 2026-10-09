# Производственный запуск TAIGA-IACT (Конвертация + Оптика + Триггер) через HTCondor

Автоматизированный сквозной расчёт полного цикла моделирования телескопов комплекса TAIGA-IACT: один исходный CORSIKA-файл — одна задача HTCondor — одно процессорное ядро (CPU).

```text
CORSIKA -> hybrid -> промежуточный _i -> TAIGA_optics_file -> промежуточный _feb -> trg5 (триггер + NSB + клининг) -> Hillas CSV
                                        (внутри изолированного scratch рабочего узла, с очисткой временных файлов)
```

---

## 1. Документация комплекса

В репозиторий включены подробные руководства для различных задач:
- **[README_FULL.md](README_FULL.md)** — **Полное техническое руководство**: архитектура распределённого конвейера HTCondor, форматы данных, изоляция scratch, управление ресурсами.
- **[README_QUICKSTART.md](README_QUICKSTART.md)** — **Краткое руководство**: выжимка команд для сборки `runtime.tar.gz`, отправки задач в очередь и мониторинга.
- **[PHYSICS_SIMULATION_GUIDE.md](../docs/PHYSICS_SIMULATION_GUIDE.md)** — **Физическое описание всех стадий**: оптическая система Дэвиса-Коттона, конусы Уинстона, квантовая эффективность ФЭУ XP1911, электроника триггера, шум ночного неба NSB и параметры Хилласа.
- **[trg5_git/README.md](../trg5_git/README.md)** — **Руководство по модулю триггера `trg5`**: параметры `trigger_config.json`, автономная компиляция и калибровочные файлы.
- **[GITLAB_MR_DESCRIPTION.md](../GITLAB_MR_DESCRIPTION.md)** — **Описание выполненной интеграции для GitLab**: аннотация, решённые проблемы, сравнение «до/после» и чеклист.

---

## 2. Ключевые архитектурные решения

### 2.1. Нулевые внешние зависимости на рабочих узлах (Zero-Dependency)
- **C++ модули (`trigger_iact`, `cleaning`)**:
  - В программу `trigger_iact` (`readbin_v4.cpp`) встроен собственный алгоритм субсплайновой интерполяции Акимы (Akima sub-spline). Для компиляции и работы **не требуются CERN ROOT или заголовочные файлы GSL** — достаточно стандартного компилятора C++11 и флага `-fopenmp`.
- **Python-оркестратор на узле (`run_trg_step.py`)**:
  - Полностью переписан на **чистый стандартный Python 3** (модули `math`, `csv`, `json`, `os`, `sys`, `pathlib`, `struct`).
  - **Не требует `numpy` или `pandas`** на слотах HTCondor, что предотвращает ошибку `ModuleNotFoundError: No module named 'numpy'`.

### 2.2. Оптимизация дискового пространства и сетевого трафика
- На рабочий узел передаются только входной файл CORSIKA и компактный архив `runtime.tar.gz`.
- Промежуточные файлы `intermediate_i` и `intermediate_t` удаляются сразу после завершения оптики.
- Промежуточный бинарный файл `result_feb` (достигающий нескольких гигабайт) обрабатывается триггером в локальной директории scratch и по умолчанию удаляется (`keep_feb=0`), освобождая дисковое пространство узла.
- На управляющий сервер (submit-хост) возвращаются только сжатые научные данные:
  - `*_trg.tar.gz` — архив очищенных событий и данных триггера;
  - `*_hillas_iact*.csv` — таблицы параметров Хилласа для разделения гамма/адрон;
  - `*_A_sums2` — диагностика оптического моделирования;
  - `*.done` — маркер успешного завершения задачи.

### 2.3. Интеллектуальный поиск калибровок
- В скрипт `run_trg_step.py` встроен многоуровневый алгоритм поиска калибровочных файлов (`Calibration/slow_pulse_iact2_new`, файлы коэффициентов, геометрии и амплитуд `probablies8.txt`). Поиск автоматически находит файлы как по прямым относительным путям, так и рекурсивно внутри директории `runtime/`.

---

## 3. Быстрый запуск

### 3.1. Сборка архива окружения `runtime.tar.gz`

Для создания самодостаточного архива выполните команду в папке `condor_pipeline`:

```bash
python3 pack_runtime.py \
    --hybrid /path/to/optics/io_taiga/hybrid \
    --optics /path/to/optics/condor_pipeline/TAIGA_optics_file \
    --assets /path/to/optics/condor_pipeline/runtime/assets \
    --library /path/to/libtaiga_io_eventio.so \
    --library /path/to/libtaiga_io_text.so \
    --library /usr/lib64/libgsl.so.25 \
    --library /usr/lib64/libgslcblas.so.0 \
    --trigger /path/to/optics/trg5_git/trigger_iact \
    --cleaning /path/to/optics/trg5_git/cleaning \
    --trg-scripts /path/to/optics/trg5_git \
    --trg-assets /path/to/optics/trg5_git \
    --amplitudes-file /path/to/probablies8.txt \
    --output runtime.tar.gz
```

*Важно:* Файл одноэлектронных амплитуд `probablies8.txt` (~198 МБ) передаётся через флаг `--amplitudes-file` и упаковывается внутрь архива.

### 3.2. Подготовка и запуск расчётной кампании в HTCondor

```bash
# 1. Генерация файлов задач для тестового события (--limit 1)
python3 prepare_run.py /path/to/corsika_data "$HOME/taiga_runs/test01" \
    --limit 1 \
    --radius 100

# 2. Переход в каталог кампании
cd "$HOME/taiga_runs/test01"

# 3. Отправка в очередь HTCondor
condor_submit pipeline.sub

# 4. Мониторинг исполнения
condor_q
tail -f logs/*-0.out
```

После завершения задачи в каталоге будут созданы итоговые файлы:
```text
847419-0.out            # Лог выполнения (этапы START -> CONVERT -> OPTICS -> TRIGGER -> FINISH)
847419-0.err            # Лог ошибок (пустой при успешном завершении)
847419-0_trg.tar.gz     # Результаты триггера, шума неба и параметров Хилласа
847419-0_A_sums2        # Диагностический файл оптического моделирования
847419-0.done           # Маркер успешного завершения задачи
```
