# OS Process Scheduling Simulator

This project implements and compares four classic CPU scheduling algorithms — **Round Robin**, **First-Come, First-Served (FCFS)**, **Shortest Job First (SJF)**, and **Multi-Level Feedback Queue (MLFQ)** — using UNIX signals and process control in C.  
It was developed as part of *COMPE571 – Embedded Operating Systems (Fall 2025)*.

---

## 📂 Project Overview
Each scheduler runs four child processes performing a CPU-bound workload (`myfunction()`), simulating process execution under different scheduling policies.  
The schedulers measure:
- Individual and average response times
- Context-switch overhead (for Round Robin and MLFQ)
- Statistical variation across multiple trials

The experiments follow a structured design to find the optimal quantum values for Round Robin and MLFQ and compare all algorithms using both varying and equal workloads.

---

## ⚙️ Implemented Algorithms

| File | Algorithm | Description |
|------|------------|--------------|
| `RR_refined.c` | Round Robin | Time-sliced scheduling with configurable quantum (`-q <µs>`). Measures context-switch overhead. |
| `FCFS_refined.c` | First-Come, First-Served | Runs tasks in creation order with no preemption. Reports response times and average context-switch gap. |
| `SJF_refined.c` | Shortest Job First | Sorts processes by workload size and executes shortest jobs first. Non-preemptive. |
| `MLFQ_refined.c` | Multi-Level Feedback Queue | Two-level queue: Round Robin (top) + FCFS (bottom). Moves unfinished jobs down after their time slice. |

---

## 🧪 Experiment Scripts

| Script | Description |
|---------|--------------|
| `run_case3a.sh` | Runs all schedulers with default workloads (Case 3a). Outputs average response times for each algorithm. |
| `run_case3b.sh` | Runs all schedulers with equal workloads (Case 3b). Calculates mean and standard deviation of response times. |
| `run_case3b_ctx.sh` | Extension of Case 3b script. Collects additional context-switch overhead data for extra-credit analysis. |

---

## 🛠️ Compilation and Usage

Compile manually:
```bash
gcc -O2 -Wall -Wextra RR_refined.c -o RR_refined
gcc -O2 -Wall -Wextra FCFS_refined.c -o FCFS_refined
gcc -O2 -Wall -Wextra SJF_refined.c -o SJF_refined
gcc -O2 -Wall -Wextra MLFQ_refined.c -o MLFQ_refined
