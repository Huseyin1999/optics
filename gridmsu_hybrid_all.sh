#!/bin/bash
runnum=$1
Nr=$2
for i in `seq 0 $((Nr-1))`; do
  let Rn=$runnum+$i
  ./hybrid /k3/iact_pool/Irkutsk-MC/sim_corsika_HiSCORE_jun2022/corsika/taiga${Rn}_iact.corsika /k3/iact_pool/corsika_intermediate/taigaHS${Rn} 100
done
