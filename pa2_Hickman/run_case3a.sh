#!/usr/bin/env bash
set -euo pipefail


RR_Q=4000       # best RR quantum from Experiment 1
MLFQ_Q=20000    # best MLFQ first-queue quantum from Experiment 2
TRIALS=5        # number of runs per algorithm

OUT="case3a_results.csv"


gcc -O2 -Wall -Wextra RR_refined.c   -o RR_refined
gcc -O2 -Wall -Wextra FCFS_refined.c -o FCFS_refined
gcc -O2 -Wall -Wextra SJF_refined.c  -o SJF_refined
gcc -O2 -Wall -Wextra MLFQ_refined.c -o MLFQ_refined


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
    # Run and capture the line with "Average Response Time"
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

# Print mean per algorithm
echo "---- Means per algorithm ----"
awk -F, 'NR>1{sum[$1]+=$3; n[$1]++} END{for(a in sum) printf "%s,%.3f\n", a, sum[a]/n[a]}' "$OUT" \
  | sort
