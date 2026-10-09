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
  #./signal /k38/taiga_pool/CORSIKA_TAIGA/mc/all1006/bpe%d_31_da5.0_md5_a2021 /taiga%d_hs 1006 100e-9 100 10 5
  #bpe1438_33_da0.0_md5_T100-40
  ./signal $basedir"mc/all$runnum/bpe%d_33_da0.0_md5" "/taiga%d_hs" "$Rn" "100e-9" "100" "10" "5" > "${dir}s_log_hs"$Rn".txt" 2> "${dir}s_errlog_hs"$Rn".txt"
  echo "$Rn done"
  sleep 1
done
