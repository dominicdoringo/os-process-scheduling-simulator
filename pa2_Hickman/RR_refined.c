/* RR_refined.c */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

/*
***********************************************************************************
*************
These DEFINE statements represent the workload size of each task and
the time quantum values for Round Robin scheduling for each task.
***********************************************************************************
**************/
#define WORKLOAD1 100000
#define WORKLOAD2 50000
#define WORKLOAD3 25000
#define WORKLOAD4 10000
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
    int alive;
    int started;
    struct timeval start_t, end_t;
} Proc;

static inline long long to_us(struct timeval t) {
    return (long long)t.tv_sec * 1000000LL + (long long)t.tv_usec;
}

int main(int argc, char *argv[]) {
    long quantum = 1000;
    if (argc == 3 && strcmp(argv[1], "-q") == 0) {
        long q = atol(argv[2]);
        if (q > 0) quantum = q;
    }

    const int workloads[NPROCS] = {WORKLOAD1, WORKLOAD2, WORKLOAD3, WORKLOAD4};
    Proc procs[NPROCS];
    int i;

    for (i = 0; i < NPROCS; i++) {
        pid_t p = fork();
        if (p == 0) {
            myfunction(workloads[i]);
            exit(0);
        }
        kill(p, SIGSTOP);
        procs[i].pid = p;
        procs[i].alive = 1;
        procs[i].started = 0;
    }

    int alive_count = NPROCS, idx = 0;
    struct timeval t1, t2;
    long long sw_overhead_sum = 0;
    int sw_count = 0, ctx_switches = 0;

    while (alive_count > 0) {
        if (!procs[idx].alive) {
            idx = (idx + 1) % NPROCS;
            continue;
        }

        if (!procs[idx].started) {
            gettimeofday(&procs[idx].start_t, NULL);
            procs[idx].started = 1;
        }

        gettimeofday(&t1, NULL);
        kill(procs[idx].pid, SIGCONT);
        usleep((useconds_t)quantum);
        kill(procs[idx].pid, SIGSTOP);
        gettimeofday(&t2, NULL);

        long long slice_us = to_us(t2) - to_us(t1);
        long long overhead = slice_us > quantum ? (slice_us - quantum) : 0;
        sw_overhead_sum += overhead;
        sw_count++;
        ctx_switches++;

        for (i = 0; i < NPROCS; i++) {
            if (procs[i].alive) {
                int status;
                pid_t r = waitpid(procs[i].pid, &status, WNOHANG);
                if (r == procs[i].pid) {
                    procs[i].alive = 0;
                    gettimeofday(&procs[i].end_t, NULL);
                    alive_count--;
                }
            }
        }

        idx = (idx + 1) % NPROCS;
    }

    long long sum_resp = 0;
    for (i = 0; i < NPROCS; i++) {
        long long rt = to_us(procs[i].end_t) - to_us(procs[i].start_t);
        sum_resp += rt;
        printf("Process %d Response Time: %lld us\n", i + 1, rt);
    }

    double avg_resp = (double)sum_resp / NPROCS;
    double avg_sw = (sw_count > 0) ? (double)sw_overhead_sum / sw_count : 0.0;

    printf("Context Switches: %d\n", ctx_switches);
    printf("Average Response Time: %.2f us\n", avg_resp);
    printf("Average Context Switch Overhead: %.3f us\n", avg_sw);

    return 0;
}
