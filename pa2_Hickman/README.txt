Overview

This project implements and compares four user-level CPU scheduling algorithms using UNIX signals and timing functions in C.
Each program creates four child processes that execute a CPU-bound workload (myfunction()), then applies a specific scheduling policy to control which process runs.

All implementations are based on the provided sample program.
Processes are created, immediately paused using SIGSTOP, and then resumed using SIGCONT according to the selected scheduling algorithm.
Each scheduler reports individual and average response times, and some also record context-switch overhead for the extra-credit requirement.

File Descriptions

RR_refined.c
Implements the Round Robin (RR) scheduler.
Measures average response time and context-switch overhead.
Used in Experiment 1 and Case 3a.

FCFS_refined.c
Implements the First-Come, First-Served (FCFS) scheduler.
Runs processes to completion in creation order with no preemption.
Reports individual and average response times and average context-switch gap.
Used in Case 3a and Case 3b.

SJF_refined.c
Implements the Shortest Job First (SJF) scheduler.
Sorts processes by workload size before execution.
Runs each process to completion in ascending workload order.
Used in Case 3a and Case 3b.

MLFQ_refined.c
Implements the Multi-Level Feedback Queue (MLFQ) scheduler with two queues:
a Round Robin queue (configurable quantum) and a lower FCFS queue.
Reports response times and context-switch gaps for each queue.
Used in Experiment 2, Case 3a, and Case 3b.

Experiment Scripts

run_case3a.sh
Automates Case 3a (default workloads).
Runs all four algorithms multiple times and records average response times.

run_case3b.sh
Automates Case 3b (equal workloads).
Runs all algorithms with workloads set to 100000 each.
Reports mean and standard deviation of average response times.

run_case3b_ctx.sh
Extended version of Case 3b script.
Also collects context-switch overhead data for each scheduler.
Used for the extra-credit analysis.

Compilation and Usage

To build any scheduler manually:
gcc -O2 -Wall -Wextra <scheduler_name>.c -o <scheduler_name>

To run a scheduler manually:
./RR_refined -q 4000
./FCFS_refined
./SJF_refined
./MLFQ_refined -q 20000

To execute experiments automatically:
./run_case3a.sh (default workloads)
./run_case3b.sh (equal workloads, 10 trials)
./run_case3b_ctx.sh (equal workloads + context switch stats)

Each script outputs results to a CSV file in the current directory:
case3a_results.csv, case3b_results.csv, case3b_ctx.csv