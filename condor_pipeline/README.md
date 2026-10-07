# Производственный запуск оптики через HTCondor

Основной режим: один исходный CORSIKA-файл — одна задача — один CPU.

```text
CORSIKA -> hybrid -> промежуточный _i -> TAIGA_optics_file -> _feb
           внутри одной временной папки рабочего узла
```

Контейнер не используется. Condor передаёт входной файл и runtime.tar.gz.
Архив распаковывается один раз. Обе программы выполняются последовательно.
Промежуточные _i и _t не возвращаются; их удаляет Condor вместе со scratch.
Возвращаются фотоэлектроны, диагностика и маркер успешного завершения.
При ошибке задание попадает в Hold; повторный запуск выполняет обе стадии.
Это исключает второе ожидание очереди и передачу/хранение промежуточных файлов.

## Готовый комплект и требования

Распакуйте condor_pipeline_release.tar.gz в собственную папку на submit-сервере:

```bash
mkdir -p "$HOME/taiga_tools"
tar -xzf /path/to/condor_pipeline_release.tar.gz -C "$HOME/taiga_tools"
cd "$HOME/taiga_tools/condor_pipeline"
```

Внутри уже есть runtime.tar.gz с hybrid, TAIGA_optics_file, четырьмя библиотеками
io_taiga/GSL, параметрами и таблицами. Комплект рассчитан на Linux x86_64 EL9
и совместимые системные glibc/libstdc++. Python >=3.9 нужен на submit-сервере,
а на worker нужны bash и tar. У отправителя должен быть доступ к accounting_group=taiga.
Не используйте чужие закрытые домашние папки для результатов.

В архиве текущая конфигурация parameters.txt/config.txt, использованная в тесте:
da=5 градусов, wobble выключен, mirror_q=0.9, track_h=0, SPM=0.
Это конкретная конфигурация, не универсальный выбор для любой физической задачи.
Проверьте её перед массовым расчётом. Необязательный mirrors.txt в тестовом
комплекте отсутствует: старый код использует нулевые смещения зеркал.

## Основной запуск

Создайте отдельную кампанию. Скрипт только готовит файлы, ничего не отправляет:

```bash
python3 prepare_run.py /data/corsika "$HOME/taiga_runs/production01"
cd "$HOME/taiga_runs/production01"
condor_submit -dry-run /dev/null pipeline.sub
condor_submit pipeline.sub
```

python3 prepare_run.py /taiga/huseyin/1000m/E_200_2000TeV/test_3PeV "/taiga/huseyin/taiga_runs/production01"



Каждая строка jobs.tsv — один job. Путь к данным должен быть читаемым на
submit-сервере; доступ worker к /taiga не требуется. Пути без пробелов и
специальных символов. Поиск по умолчанию *_iact.corsika, без рекурсии.
Для первого запуска рекомендуется --limit 1 в prepare_run.py.
Другой набор входов: --pattern '*.corsika'. Радиус разделения: --radius 100 (см).

В папке кампании:

- runtime.tar.gz — собственная копия окружения;
- pipeline.sub, pipeline_one.sh — описание и worker;
- campaign.json — настройки и SHA-256 runtime;
- jobs.tsv — список задач;
- logs — stdout, stderr и Condor event log каждой задачи;
- results — PREFIX_feb, PREFIX_A_sums2 и PREFIX.done.

PREFIX — имя CORSIKA без .corsika. Суффикс результата определяется автоматически
из SPM/track_h упакованного файла параметров.
Пустой _t допустим для выборки только с IACT; он не используется оптикой.

## Контроль, ошибки и продолжение

```bash
condor_q CLUSTER
condor_q CLUSTER -hold
tail -n 60 logs/CLUSTER-0.log
cat logs/CLUSTER-0.err
tail -n 40 logs/CLUSTER-0.out
ls -lh results/
```

Worker печатает START/CONVERT/OPTICS/FINISH с UTC-временем. Ожидание Condor
не входит во время программы. Наличие .done и exit-code 0 означают успешное
выполнение проверок, но не заменяют научную проверку результата.

Повторите prepare_run.py с теми же аргументами ПОСЛЕ завершения/удаления предыдущей
очереди: уже готовые файлы будут пропущены. Маркер содержит ключ входа
(путь, размер, mtime), runtime и настроек; наличие всех результатов проверяется.
После теста --limit 1 уберите --limit и подготовьте оставшиеся задания.
--force включает готовые входы повторно. Не отправляйте один вход в тот же
output prefix одновременно из двух очередей.
Изменились runtime/параметры/radius — создайте новую кампанию.
Для больших файлов увеличьте request_memory/request_disk в копии pipeline.sub;
это лимиты одного задания (по умолчанию 8GB RAM, 4GB scratch).

## Отдельные этапы для отладки

Остаются convert.sub/convert_one.sh и simulate.sub/simulate_one.sh:

```bash
cd "$HOME/taiga_tools/condor_pipeline"
mkdir -p logs/convert logs/simulate converted results
python3 prepare_jobs.py convert /data/corsika "$PWD/converted"
condor_submit convert.sub
tar -xzf runtime.tar.gz runtime/assets
python3 prepare_jobs.py simulate "$PWD/converted" "$PWD/results" \
  --parameters "$PWD/runtime/assets/parameters.txt"
condor_submit simulate.sub
```

Отладочный генератор имеет более простую проверку актуальности: после изменения
архива или настроек используйте --force. Для track_h=1 передайте --suffix=_fhb;
при SPM=1 — _seb/_shb. Для другой конфигурации parameters_name=ИМЯ.txt в submit.
Отдельные стадии полезны для многократного расчёта разных камер по одному _i.

## Изменение конфигурации и сборка

Редактирование исходных таблиц рядом с программой не меняет готовый runtime.
Распакуйте архив в отдельной папке, измените runtime/assets и соберите новый
runtime.tar.gz через pack_runtime.py. Все ссылки на таблицы должны быть
относительными внутри assets. Данные .txt/.dat копируются с подкаталогами;
другие типы файлов упаковщик не включает.

```bash
python3 pack_runtime.py --hybrid /path/to/hybrid \
  --optics /path/to/TAIGA_optics_file --assets /path/to/assets \
  --library /path/to/libtaiga_io_eventio.so \
  --library /path/to/libtaiga_io_text.so \
  --library /path/to/libgsl.so.25 --library /path/to/libgslcblas.so.0
python3 package_release.py
```

pack_runtime.py проверяет ldd и сохраняет SHA-256 файлов в manifest.json.
Для компиляции оптики: make в этой папке (нужен GSL-devel).
hybrid собирается из io_taiga с CORSIKADIR, содержащим bernlohr.
Исходники старых программ не изменяются; direct_main.cpp добавляет файловый CLI.
Архивы бинарников исключены из Git и передаются отдельно от исходников.
Используйте только доверенные runtime-архивы.



python3 pack_runtime.py \
  --hybrid /taiga/huseyin/optics/io_taiga \
  --optics /taiga/huseyin/optics/TAIGA_optics \
  --assets /taiga/huseyin/optics/TAIGA_optics \
  --library ./native \
  --output runtime.tar.gz
