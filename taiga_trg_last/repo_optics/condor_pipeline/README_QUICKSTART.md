# Краткая инструкция по сборке, контейнеризации и запуску TAIGA condor_pipeline

Краткое практическое руководство: как собрать исполняемые модули, упаковать `runtime.tar.gz`, собрать контейнер и запустить сквозной расчёт в HTCondor.

---

## 1. Сборка программ

### 1.1. Компиляция оптики и конвертера
```bash
# Сборка hybrid (из io_taiga)
cd /path/to/optics/io_taiga
make

# Сборка TAIGA_optics_file
cd /path/to/optics/condor_pipeline
make
```

### 1.2. Компиляция триггера и клининга (`trg5`)
В актуальной версии `readbin_v4.cpp` реализован собственный высокоскоростной алгоритм сплайнов Акимы. **Библиотеки CERN ROOT и GSL-заголовки больше не требуются!** Достаточно стандартного компилятора C++11 с поддержкой OpenMP:

```bash
cd /path/to/optics/trg5_git

# Сборка триггера и клининга через Makefile
make

# Или вручную отдельными командами:
# Сборка клининга:
g++ -std=c++11 -O3 cleaning_corsika_pipeline.cpp -o cleaning -Wall

# Сборка триггера (без ROOT и GSL):
g++ -std=c++11 -O3 readbin_v4.cpp -o trigger_iact -fopenmp
```
*Примечание:* Для сборки с CERN ROOT доступна отдельная цель `make trigger_root`, однако для запуска в HTCondor рекомендуется стандартная сборка без внешних зависимостей.

---

## 2. Упаковка `runtime.tar.gz`

Сборка самодостаточного архива окружения со всеми бинарниками, калибровками и крупным файлом амплитуд `probablies8.txt`:

```bash
cd /path/to/optics/condor_pipeline

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

> [!IMPORTANT]
> Файл `probablies8.txt` не хранится в Git из-за большого размера. Укажите реальный путь к нему на submit-сервере через аргумент `--amplitudes-file`. Скрипт упакует его внутрь `runtime/assets/trg5/probablies8.txt`.

---

## 3. Сборка и запуск в контейнере (Apptainer / Singularity — опционально)

Благодаря переводу конвейера `run_trg_step.py` на стандартный Python 3 (без `numpy`/`pandas`) и устранению зависимостей от ROOT и GSL-заголовков, **контейнер больше не является обязательным**. Задания успешно выполняются напрямую на рабочих узлах HTCondor с использованием самодостаточного архива `runtime.tar.gz`.

Однако, если в вашей гетерогенной вычислительной среде требуется полная изоляция системных библиотек, доступен готовый рецепт контейнера.

### 3.1. Сборка образа SIF (на сервере с правами root или fakeroot):
```bash
cd /path/to/optics/condor_pipeline/container
apptainer build taiga_runtime.sif taiga_runtime.def
```

### 3.2. Использование контейнера в HTCondor:
Для запуска через контейнер добавьте в `pipeline.sub`:
```text
+SingularityImage = "/path/to/containers/taiga_runtime.sif"
```
По умолчанию конвейер запускается напрямую без контейнера, используя стандартный `python3` и библиотеки из `runtime.tar.gz`.

---

## 4. Запуск кампании в HTCondor

### 4.1. Подготовка кампании (тестовый прогон 1 события)
Перед отправкой тысяч задач всегда проверяйте корректность на одном файле:

```bash
# 1. Создание папки кампании с лимитом в 1 задачу
python3 prepare_run.py /path/to/corsika_data "$HOME/taiga_runs/test01" \
    --limit 1 \
    --radius 100

# 2. Переход в папку кампании
cd "$HOME/taiga_runs/test01"

# 3. Сухая проверка синтаксиса HTCondor
condor_submit -dry-run /dev/null pipeline.sub

# 4. Отправка в очередь
condor_submit pipeline.sub
```

### 4.2. Мониторинг и контроль тестовой задачи
```bash
# Просмотр очереди задач
condor_q

# Просмотр логов выполнения
tail -f logs/*-0.out
cat logs/*-0.err

# Проверка полученных результатов
ls -lh results/
```
В каталоге `results/` должны появиться:
- `PREFIX_trg.tar.gz` — архив результатов триггера, фона и клининга;
- `PREFIX_hillas_iact0*.csv` — таблицы параметров Хилласа;
- `PREFIX_A_sums2` — оптическая диагностика;
- `PREFIX.done` — маркер успешного завершения.

### 4.3. Массовый производственный запуск
После успешного теста вернитесь и подготовьте полный список задач:

```bash
# Запуск для всех файлов папки (уже готовое тестовое событие автоматически пропустится)
python3 prepare_run.py /path/to/corsika_data "$HOME/taiga_runs/production01" \
    --radius 100

cd "$HOME/taiga_runs/production01"
condor_submit pipeline.sub
```

---

## 5. Полезные опции и флаги `prepare_run.py`

| Флаг | Значение по умолчанию | Описание |
|---|---|---|
| `--limit N` | Все файлы | Ограничить число подготовленных задач числом N |
| `--keep-feb` | Отключен (0) | Сохранять бинарный FEB-файл на submit-сервере (по умолчанию удаляется для экономии диска) |
| `--radius R` | `100` (см) | Радиус отбора фотонов вокруг оптической оси телескопа в `hybrid` |
| `--force` | Выключен | Принудительно перезапустить все задачи, игнорируя маркеры `.done` |
| `--trigger-config` | Встроенный | Задать собственный JSON-файл параметров триггера и порогов клининга |
| `--pattern` | `*_iact.corsika` | Маска поиска входных файлов CORSIKA |

---

## 6. Диагностика ошибок и повторный запуск

- **Задачи в состоянии `Hold`**:
  ```bash
  condor_q -hold
  ```
- **Очистка очереди при ошибке**:
  ```bash
  condor_rm CLUSTER_ID
  ```
- **Возобновление после исправления**:
  Просто повторите команду `prepare_run.py` с теми же аргументами. Успешно завершённые задачи (имеющие файл `.done` и готовые результаты) будут автоматически пропущены, а упавшие или неначатые задачи будут добавлены в новый `jobs.tsv` для повторной отправки.
