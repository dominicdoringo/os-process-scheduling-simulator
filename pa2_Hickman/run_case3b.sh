#!/usr/bin/env bash
set -euo pipefail

# ---- CONFIGURE ----
TRIALS=10             # repeat enough times for stable stats
RR_Q=4000             #  best RR from Exp 1 
MLFQ_Q=20000          #  best MLFQ from Exp 2 

# All equal workloads for Case 3b:
W=100000
WDEFS="-DWORKLOAD1=$W -DWORKLOAD2=$W -DWORKLOAD3=$W -DWORKLOAD4=$W"

OUT="case3b_results.csv"

# Build fresh with equal workloads
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

run_algo() {
  local name="$1"; shift
  local cmd="$*"
  local runner
  runner=$(maybe_pin "$cmd")
  for t in $(seq 1 "$TRIALS"); do
    line=$($runner | grep -E "Average Response Time")
    avg=$(echo "$line" | awk '{match($0, /([0-9]+(\.[0-9]+)?)/, m); print m[1]}')
    echo "$name,$t,$avg" | tee -a "$OUT"
    sleep 0.05
  done
}

echo "algo,trial,avg_response_us" > "$OUT"
run_algo RR    "./RR_refined -q $RR_Q"
run_algo FCFS  "./FCFS_refined"
run_algo SJF   "./SJF_refined"
run_algo MLFQ  "./MLFQ_refined -q $MLFQ_Q"

# Print mean and std dev per algorithm (Welford's algorithm in awk)
echo "---- Means and StdDev per algorithm ----"
awk -F, 'NR>1{
  a=$1; x=$3+0;
  n[a]++; d=x - mean[a]; mean[a]+= d/n[a]; m2[a]+= d*(x-mean[a]);
}
END{
  for (a in n) {
    var = (n[a]>1)? m2[a]/(n[a]-1) : 0;
    sd  = sqrt(var);
    printf "%s,mean=%.3f,sd=%.3f,n=%d\n", a, mean[a], sd, n[a];
  }
}' "$OUT" | sort
