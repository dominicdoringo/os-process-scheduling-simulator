/* MLFQ_refined.c */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>

/*
***********************************************************************************
*************
These DEFINE statements represent the workload size of each task and
the time quantum values for the first queue in MLFQ scheduling.
***********************************************************************************
**************/
#define WORKLOAD1 100000
#define WORKLOAD2 50000
#define WORKLOAD3 25000
#define WORKLOAD4 10000
#define DEFAULT_QUANTUM 20000
#define NPROCS 4

/*
***********************************************************************************
*************
DO NOT CHANGE THE FUNCTION IMPLEMENTATION
***********************************************************************************
**************/
void myfunction(int param) {
    int i = 2;
    int j, k;
    while (i < param) {
        k = i;
        for (j = 2; j <= k; j++) {
            if (k % j == 0) {
                k = k / j;
                j--;
                if (k == 1) break;
            }
        }
        i++;
    }
}

/*
***********************************************************************************
*************/
typedef struct {
    pid_t pid;
    int started;
    struct timespec t0, tf;
} Proc;

static inline long long usec_ts(struct timespec ts) {
    return (long long)ts.tv_sec * 1000000LL + (long long)(ts.tv_nsec / 1000);
}

static long parse_q(int argc, char **argv, long defq) {
    for (int i = 1; i + 1 < argc; i++) {
        if (!strcmp(argv[i], "-q")) {
            long v = atol(argv[i + 1]);
            if (v > 0) return v;
        }
    }
    return defq;
}

int main(int argc, char *argv[]) {
    long quantum = parse_q(argc, argv, DEFAULT_QUANTUM);
    printf("First-queue Quantum: %ld us\n", quantum);

    Proc procs[NPROCS];
    int workloads[NPROCS] = {WORKLOAD1, WORKLOAD2, WORKLOAD3, WORKLOAD4};
    int q1[NPROCS], q2[NPROCS];
    int q1_len = 0, q2_len = 0;

    for (int i = 0; i < NPROCS; i++) {
        pid_t p = fork();
        if (p == 0) { myfunction(workloads[i]); _exit(0); }
        kill(p, SIGSTOP);
        procs[i].pid = p;
        procs[i].started = 0;
        q1[q1_len++] = i;
    }

/**********************************************************************************
**************
At this point, all newly-created child processes are stopped, and
ready for scheduling.
***********************************************************************************
**************/
/**********************************************************************************
**************
- Scheduling code starts here
***********************************************************************************
**************/
    struct timespec ctx_a, ctx_b;
    long long rr_ctx_sum = 0, fcfs_ctx_sum = 0;
    int rr_ctx_cnt = 0, fcfs_ctx_cnt = 0;

    /* Q1: one RR slice per process; unfinished go to Q2 */
    clock_gettime(CLOCK_MONOTONIC, &ctx_a);
    for (int k = 0; k < q1_len; k++) {
        int i = q1[k];

        if (!procs[i].started) { clock_gettime(CLOCK_MONOTONIC, &procs[i].t0); procs[i].started = 1; }

        kill(procs[i].pid, SIGCONT);
        clock_gettime(CLOCK_MONOTONIC, &ctx_b);
        long long gap = usec_ts(ctx_b) - usec_ts(ctx_a);
        printf("Context switch: %lld us\n", gap);
        rr_ctx_sum += gap; rr_ctx_cnt++;

        /* RR slice */
        usleep((useconds_t)quantum);
        kill(procs[i].pid, SIGSTOP);

        int status;
        pid_t r = waitpid(procs[i].pid, &status, WNOHANG);
        if (r > 0) {
            clock_gettime(CLOCK_MONOTONIC, &procs[i].tf);
            long long rt = usec_ts(procs[i].tf) - usec_ts(procs[i].t0);
            printf("Response Time (Process %d): %lld us\n", i + 1, rt);
        } else {
            q2[q2_len++] = i; /* demote to FCFS */
        }
        clock_gettime(CLOCK_MONOTONIC, &ctx_a);
    }

    /* Q2: FCFS to completion in arrival order from Q1 */
    if (q2_len > 0) {
        clock_gettime(CLOCK_MONOTONIC, &ctx_a);
        for (int k = 0; k < q2_len; k++) {
            int i = q2[k];

            kill(procs[i].pid, SIGCONT);
            clock_gettime(CLOCK_MONOTONIC, &ctx_b);
            long long gap2 = usec_ts(ctx_b) - usec_ts(ctx_a);
            printf("Context switch (FCFS): %lld us\n", gap2);
            fcfs_ctx_sum += gap2; fcfs_ctx_cnt++;

            waitpid(procs[i].pid, NULL, 0);
            clock_gettime(CLOCK_MONOTONIC, &procs[i].tf);
            long long rt = usec_ts(procs[i].tf) - usec_ts(procs[i].t0);
            printf("Response Time (Process %d): %lld us\n", i + 1, rt);

            clock_gettime(CLOCK_MONOTONIC, &ctx_a);
        }
    }
/**********************************************************************************
**************
- Scheduling code ends here
***********************************************************************************
**************/
    long long total = 0;
    for (int i = 0; i < NPROCS; i++) {
        long long rt = usec_ts(procs[i].tf) - usec_ts(procs[i].t0);
        total += rt;
    }
    double avg_resp = (double)total / NPROCS;
    printf("Average Response Time: %.2f us\n", avg_resp);

    if (rr_ctx_cnt > 0)  printf("Avg RR context switch gap: %.2f us\n", (double)rr_ctx_sum / rr_ctx_cnt);
    if (fcfs_ctx_cnt > 0) printf("Avg FCFS context switch gap: %.2f us\n", (double)fcfs_ctx_sum / fcfs_ctx_cnt);

    return 0;
}
