#!/bin/bash


export CORSIKADIR=/media/husein/288795a8-ccc3-4052-bc28-3e3009f96477/husein/CORSIKA/corsika-73500
export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
# путь к программе
HYBRID_DIR="/media/husein/288795a8-ccc3-4052-bc28-3e3009f96477/husein/TAIGA/optics/io_taiga"

# папка с исходными файлами
DATA_DIR="/home/husein/Cluster/taiga/huseyin/C"

# папка для результатов
OUT_DIR="/home/husein/Cluster/taiga/huseyin/post_io/post_io_C"

# перейти в директорию с данными
cd "$DATA_DIR" || exit

for f in *_iact.corsika
do
    name=${f%.corsika}

    echo "Processing $f"

    # запуск конвертации
    "$HYBRID_DIR/hybrid" "$f" "${name}_split" 100

    # перемещение результатов
    mv ${name}_split_* "$OUT_DIR/"
done


