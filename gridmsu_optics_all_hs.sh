#!/bin/bash
runnum=$1
Nr=$2
basedir0="/k38/taiga_pool/CORSIKA_TAIGA/"
basedir=$basedir0"corsika_intermediate/"
dir=$basedir0"TAIGA_optics/logs/"
#outdir=$basedir"output/taiga"
#cd $basedir"corsika-73500/run"
#postf="_SIT6"
#postf="_SQ1"
#postf="_T10"
#postf="_T120-30"
postf=""
for i in `seq 0 $((Nr-1))`; do
  let Rn=$runnum+$i
#  nohup ./corsika73500Linux_QGSII_gheisha < "$dir$Rn.inputcard" > "log$Rn.txt" 2> "errlog$Rn.txt" &
#  `sed -i '7c '"$Rn" "config.txt"`  
  nohup ./TAIGA_optics hiscore_parameters${postf}.txt $Rn > "${dir}log${postf}_"$Rn"_hs.txt" 2> "${dir}errlog${postf}_"$Rn"_hs.txt" &
  sleep 1
done
