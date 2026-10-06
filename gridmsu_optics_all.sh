#!/bin/bash
runnum=$1
Nr=$2
particlename=$3
basedir0="/media/husein/Work_Hard/TAIGA/optics/"
basedir=$basedir0"post_io_${particlename}/"
dir=$basedir0"TAIGA_optics/logs/"
#outdir=$basedir"output/taiga"
#cd $basedir"corsika-73500/run"
#postf="_SIT6"
#postf="_SQ1"
#postf="_T10"
cd ${basedir0}TAIGA_optics
postf=""
#postf="_square3"
for i in `seq 0 $((Nr-1))`; do
  let Rn=$runnum+$i
#  nohup ./corsika73500Linux_QGSII_gheisha < "$dir$Rn.inputcard" > "log$Rn.txt" 2> "errlog$Rn.txt" &
#  `sed -i '7c '"$Rn" "config.txt"`  
  nohup ./TAIGA_optics parameters${postf}.txt $Rn > "${dir}log${postf}_${Rn}.txt" 2> "${dir}errlog${postf}_${Rn}.txt" &
  sleep 1
done
