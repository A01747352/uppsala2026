/*
 * overhead_fork.c
 *
 * Forks off N processes. Each child just prints a message and exits;
 * the parent times how long it takes to create and reap all of them.
 *
 * Usage: ./overhead_fork <num_processes>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Function for tracking time
static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[]) {
    // Check the arguments
    if (argc != 2) {
        fprintf(stderr, "usage: %s <num_processes>\n", argv[0]);
        exit(1);
    }
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "num_processes must be a positive integer\n");
        exit(1);
    }

    // Allocate an array to store the PIDs of all child processes
    pid_t *pids = malloc((size_t)n * sizeof(pid_t));
    if (pids == NULL) {
        perror("malloc");
        exit(1);
    }

    double start = now();

    // Create the child processes
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child process
            printf("process created\n");
            exit(0);
        } else {
            // Parent process: keep track of PID of child so we can wait for it
            pids[i] = pid;
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < n; i++) {
        if (waitpid(pids[i], NULL, 0) < 0) {
            perror("waitpid");
            exit(1);
        }
    }

    double end = now();
    free(pids);

    double elapsed = end - start;
    printf("Created and reaped %d processes in %.6f ms (%.2f ms/processes)\n",
           n, elapsed*1000.0, elapsed*1000.0/n);

    return 0;
}
