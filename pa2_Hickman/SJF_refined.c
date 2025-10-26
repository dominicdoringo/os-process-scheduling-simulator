/* SJF_refined.c */
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
These DEFINE statements represent the workload size of each task.
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
    int workload;
    int started;
    struct timespec t0, tf;
} Proc;

static inline long long usec_ts(struct timespec ts) {
    return (long long)ts.tv_sec * 1000000LL + (long long)(ts.tv_nsec / 1000);
}

int main(void) {
    Proc p[NPROCS];
    int wl[NPROCS] = {WORKLOAD1, WORKLOAD2, WORKLOAD3, WORKLOAD4};

    for (int i = 0; i < NPROCS; i++) {
        pid_t c = fork();
        if (c == 0) { myfunction(wl[i]); _exit(0); }
        kill(c, SIGSTOP);
        p[i].pid = c;
        p[i].workload = wl[i];
        p[i].started = 0;
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
    /* sort by workload (ascending) */
    for (int i = 0; i < NPROCS - 1; i++) {
        int m = i;
        for (int j = i + 1; j < NPROCS; j++)
            if (p[j].workload < p[m].workload) m = j;
        if (m != i) { Proc tmp = p[i]; p[i] = p[m]; p[m] = tmp; }
    }

    struct timespec ctx_a, ctx_b;
    long long ctx_sum = 0;
    int ctx_cnt = 0;

    clock_gettime(CLOCK_MONOTONIC, &ctx_a);
    for (int i = 0; i < NPROCS; i++) {
        if (!p[i].started) { clock_gettime(CLOCK_MONOTONIC, &p[i].t0); p[i].started = 1; }

        kill(p[i].pid, SIGCONT);
        clock_gettime(CLOCK_MONOTONIC, &ctx_b);
        long long gap = usec_ts(ctx_b) - usec_ts(ctx_a);
        printf("Context switch: %lld us\n", gap);
        ctx_sum += gap; ctx_cnt++;

        waitpid(p[i].pid, NULL, 0);
        clock_gettime(CLOCK_MONOTONIC, &p[i].tf);

        long long rt = usec_ts(p[i].tf) - usec_ts(p[i].t0);
        printf("Process %d (workload=%d) Response Time: %lld us\n", i + 1, p[i].workload, rt);

        clock_gettime(CLOCK_MONOTONIC, &ctx_a);
    }
/**********************************************************************************
**************
- Scheduling code ends here
***********************************************************************************
**************/
    long long total = 0;
    for (int i = 0; i < NPROCS; i++) total += (usec_ts(p[i].tf) - usec_ts(p[i].t0));
    double avg_resp = (double)total / NPROCS;
    double avg_ctx  = (ctx_cnt > 0) ? (double)ctx_sum / ctx_cnt : 0.0;

    printf("Average Response Time: %.2f us\n", avg_resp);
    printf("Average Context Switch Gap: %.2f us\n", avg_ctx);

    return 0;
}
