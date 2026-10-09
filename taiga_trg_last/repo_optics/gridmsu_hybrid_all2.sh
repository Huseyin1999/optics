#!/bin/bash
runnum=$1
Nr=$2
for i in `seq 0 $((Nr-1))`; do
  let Rn=$runnum+$i
#  ./hybrid /k38/taiga_pool/CORSIKA_TAIGA/output/taiga${Rn}_iact.corsika /home/grinyuk/taiga_pool/CORSIKA_TAIGA/corsika_intermediate/taigaHS${Rn} 100
#  ./hybrid /home/grinyuk/taiga_pool/CORSIKA_TAIGA/output/taiga${Rn}_iact.corsika /home/grinyuk/taiga_pool/CORSIKA_TAIGA/corsika_intermediate/taigaHS${Rn} 100
#  ./hybrid /k38/taiga_pool/CORSIKA_TAIGA/output/taiga${Rn}_iact.corsika /k38/taiga_pool/CORSIKA_TAIGA/corsika_intermediate/taigaHS${Rn} 100
  ./hybrid /k40/taiga/SIMULATION/run${runnum}/taiga${Rn}_iact.corsika /k40/taiga/SIMULATION/run${runnum}/taigaHS${Rn} 100
done
