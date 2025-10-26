#!/usr/bin/env bash
set -euo pipefail

TRIALS=10
RR_Q=4000
MLFQ_Q=20000
W=100000
WDEFS="-DWORKLOAD1=$W -DWORKLOAD2=$W -DWORKLOAD3=$W -DWORKLOAD4=$W"

# build with equal workloads
gcc -O2 -Wall -Wextra $WDEFS RR_refined.c   -o RR_refined
gcc -O2 -Wall -Wextra $WDEFS FCFS_refined.c -o FCFS_refined
gcc -O2 -Wall -Wextra $WDEFS SJF_refined.c  -o SJF_refined
gcc -O2 -Wall -Wextra $WDEFS MLFQ_refined.c -o MLFQ_refined

maybe_pin() {
  if command -v taskset >/dev/null 2>&1; then
    echo "taskset -c 0 $*"
  else
    echo "$*"
  fi
}

OUT="case3b_ctx.csv"
echo "algo,trial,metric,value_us" > "$OUT"

run_rr() {
  local runner; runner=$(maybe_pin "./RR_refined -q $RR_Q")
  for t in $(seq 1 $TRIALS); do
    val=$($runner | awk '/Average Context Switch Overhead/ {for(i=1;i<=NF;i++) if ($i ~ /^[0-9]+(\.[0-9]+)?$/){print $i}}')
    echo "RR,$t,avg_ctx_overhead,$val" | tee -a "$OUT" >/dev/null
    sleep 0.05
  done
}

run_fcfs_or_sjf() {
  local name="$1" cmd="$2" runner
  runner=$(maybe_pin "$cmd")
  for t in $(seq 1 $TRIALS); do
    val=$($runner | awk '/Average Context Switch Gap/ {for(i=1;i<=NF;i++) if ($i ~ /^[0-9]+(\.[0-9]+)?$/){print $i}}')
    echo "$name,$t,avg_ctx_gap,$val" | tee -a "$OUT" >/dev/null
    sleep 0.05
  done
}

run_mlfq() {
  local runner; runner=$(maybe_pin "./MLFQ_refined -q $MLFQ_Q")
  for t in $(seq 1 $TRIALS); do
    rr=$($runner | awk '/Avg RR context switch gap/   {for(i=1;i<=NF;i++) if ($i ~ /^[0-9]+(\.[0-9]+)?$/){print $i}}')
    # re-run once to get FCFS gap (output order makes single run simpler; tiny extra cost)
    fcfs=$( $(maybe_pin "./MLFQ_refined -q $MLFQ_Q") | awk '/Avg FCFS context switch gap/ {for(i=1;i<=NF;i++) if ($i ~ /^[0-9]+(\.[0-9]+)?$/){print $i}}')
    echo "MLFQ,$t,avg_rr_ctx_gap,$rr"   | tee -a "$OUT" >/dev/null
    echo "MLFQ,$t,avg_fcfs_ctx_gap,$fcfs" | tee -a "$OUT" >/dev/null
    sleep 0.05
  done
}

run_rr
run_fcfs_or_sjf "FCFS" "./FCFS_refined"
run_fcfs_or_sjf "SJF"  "./SJF_refined"
run_mlfq

echo "---- Means & SD (µs) ----"
# overall per algorithm (mixing metrics when present)
awk -F, 'NR>1{a=$1; x=$4+0; n[a]++; d=x-mean[a]; mean[a]+=d/n[a]; m2[a]+=d*(x-mean[a])}
END{for(a in n){sd=(n[a]>1)?sqrt(m2[a]/(n[a]-1)):0; printf "%s,mean=%.3f,sd=%.3f,n=%d\n",a,mean[a],sd,n[a]}}' "$OUT" | sort

echo "---- MLFQ breakdown (RR vs FCFS gaps) ----"
awk -F, 'NR>1 && $1=="MLFQ"{
  k=$3; x=$4+0; n[k]++; d=x-mean[k]; mean[k]+=d/n[k]; m2[k]+=d*(x-mean[k]);
}
END{
  for(k in n){sd=(n[k]>1)?sqrt(m2[k]/(n[k]-1)):0; printf "%s,mean=%.3f,sd=%.3f,n=%d\n",k,mean[k],sd,n[k]}
}' "$OUT" | sort
