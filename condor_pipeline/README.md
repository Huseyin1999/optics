# Двухэтапный запуск оптики через HTCondor

Новая схема находится только в этой папке и не изменяет старые программы и
скрипты. Параллелизм обеспечивает HTCondor:

```text
Этап 1: один job = один *_iact.corsika
        CORSIKA eventio -> hybrid -> *_split_t + *_split_i

Этап 2: один job = один готовый *_split_i
        *_split_i -> TAIGA_optics_file -> *_feb (или другой бинарный формат)
```

Этапы не связаны DAG-зависимостью. Второй submit можно подготовить и отправить
отдельно, когда нужные конвертированные файлы уже существуют.

## 1. Сборка

Сначала собирается старый `io_taiga/hybrid`, затем отдельный файловый интерфейс
оптики из этой папки:

```bash
cd /path/to/optics/io_taiga
export CORSIKADIR=/path/to/corsika-73500
make libs hybrid

cd ../condor_pipeline
make -j"$(nproc)"
```

`TAIGA_optics_file` использует существующий код физики из `TAIGA_optics`, но
собирается отдельно и принимает явные пути входа и выхода.

## 2. Этап конвертации

Создать список заданий:

```bash
cd /taiga/huseyin/optics/condor_pipeline
mkdir -p logs/convert logs/simulate

python3 prepare_jobs.py convert \
  /data/corsika \
  /data/converted \
  --jobs-file convert_jobs.tsv
```

Каждая строка `convert_jobs.tsv` становится ровно одним Condor job. Отправить
кластер заданий:

```bash
condor_submit convert.sub hybrid=/taiga/huseyin/optics/io_taiga/hybrid corsika_dir=/taiga/huseyin/optics jobs_file=/taiga/huseyin/optics/condor_pipeline/convert_jobs.tsv
```

Параметр разделения станций по умолчанию равен 100 см. Другое значение:

```bash
condor_submit convert.sub radius=150 \
  hybrid=/path/to/hybrid \
  corsika_dir=/path/to/corsika-73500 \
  jobs_file="$PWD/convert_jobs.tsv"
```

Результаты сначала записываются во временные файлы и перемещаются на итоговые
имена только после успешной проверки обоих файлов `_t` и `_i`.

## 3. Этап моделирования оптики

После завершения нужных конвертаций создать независимый список второго этапа:

```bash
python3 prepare_jobs.py simulate \
  /data/converted \
  /data/optics-results \
  --parameters /path/to/optics/TAIGA_optics/parameters.txt \
  --jobs-file simulate_jobs.tsv
```

Отправить задания:

```bash
condor_submit simulate.sub \
  optics_executable="$PWD/TAIGA_optics_file" \
  parameters=/path/to/optics/TAIGA_optics/parameters.txt \
  jobs_file="$PWD/simulate_jobs.tsv"
```

Успешный job создаёт маркер `OUTPUT_PREFIX.done`. Подготовка нового списка
автоматически пропускает актуальные результаты. `--force` включает в список все
найденные входные файлы повторно.

## Поведение при ошибках

- stdout, stderr и Condor event log каждого задания находятся в
  `logs/convert` или `logs/simulate`;
- каждый job запрашивает ровно одно CPU;
- завершившийся с ошибкой job переводится в состояние Hold;
- частичные временные файлы удаляются worker-скриптом;
- входные и выходные каталоги должны быть видны с worker-узлов и внутри
  Singularity-контейнера;
- пути не должны содержать пробелы.

Образ контейнера и объёмы памяти можно изменить непосредственно в `convert.sub`
и `simulate.sub` либо через параметры `condor_submit`.
