#!/bin/bash
runnum=$1
Nr=$2
basedir="/k38/taiga_pool/CORSIKA_TAIGA/"
dir=$basedir"TAIGA_optics/logs/"
#outdir=$basedir"output/taiga"
#cd $basedir"corsika-73500/run"
for i in `seq 0 $((Nr-1))`; do
  let Rn=$runnum+$i
#  nohup ./corsika73500Linux_QGSII_gheisha < "$dir$Rn.inputcard" > "log$Rn.txt" 2> "errlog$Rn.txt" &
#  `sed -i '7c '"$Rn" "config.txt"`  
  ./TAIGA_optics parameters_T10.txt $Rn > "${dir}log$Rn.txt" 2> "${dir}errlog$Rn.txt"
  echo "$Rn done"
  sleep 1
done
