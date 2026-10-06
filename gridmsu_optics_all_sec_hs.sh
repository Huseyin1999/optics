#!/bin/bash
runnum=$1
Nr=$2
basedir="/k38/taiga_pool/CORSIKA_TAIGA/"
dir=$basedir"TAIGA_optics/logs/"
#outdir=$basedir"output/taiga"
#cd $basedir"corsika-73500/run"
postf="_T100-45"
#postf=""
for i in `seq 0 $((Nr-1))`; do
  let Rn=$runnum+$i
#  nohup ./corsika73500Linux_QGSII_gheisha < "$dir$Rn.inputcard" > "log$Rn.txt" 2> "errlog$Rn.txt" &
#  `sed -i '7c '"$Rn" "config.txt"`  
#  ./TAIGA_optics hiscore_parameters.txt $Rn > "${dir}log_hs"$Rn".txt" 2> "${dir}errlog_hs"$Rn".txt"
  ./TAIGA_optics hiscore_parameters${postf}.txt $Rn > "${dir}log${postf}_"$Rn"_hs.txt" 2> "${dir}errlog${postf}_"$Rn"_hs.txt"
  echo "$Rn done"
  sleep 1
done
