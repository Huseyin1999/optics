#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
runnum=20000
for f in $(seq 0 ${runnum})
do
    echo "Processing $f"
	python3 main_processing.py trigger_config.json /media/husein/288795a8-ccc3-4052-bc28-3e3009f96477/husein/TAIGA/optics/post_TAIGA_optics/post_TAIGA_optics_gamma/bpe${f}_33_da5.0_md5/taiga${f}_feb > /dev/null 

done
cd ..
cd gamma
python3 hillas_all.py