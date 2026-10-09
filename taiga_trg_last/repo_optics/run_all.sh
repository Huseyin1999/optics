#!/bin/bash

particlename='gamma1'

mkdir post_io_${particlename}
mkdir ${particlename}

cd ${particlename}
scp -r karatash@10.220.18.43:/taiga/huseyin/1000m/E_200_2000TeV/${particlename}/*_iact.corsika .
cd ..

cd io_taiga
export CORSIKADIR=/home/husein/Cluster/taiga/huseyin/optics/io_taiga
export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH

#cd ${particlename}

# путь к программе
HYBRID_DIR="/home/husein/Cluster/taiga/huseyin/optics/io_taiga"

# папка с исходными файлами
DATA_DIR="/home/husein/Cluster/taiga/huseyin/muon_chuisk_steppe/${particlename}"

# папка для результатов

OUT_DIR="/home/husein/Cluster/taiga/huseyin/optics/post_io_${particlename}"

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
cd ..

rm -r ${particlename}
mkdir post_TAIGA_optics_${particlename}
./gridmsu_optics_all.sh 0 20000 ${particlename}

rm -r post_io_${particlename}
mv post_TAIGA_optics_${particlename} post_TAIGA_optics/