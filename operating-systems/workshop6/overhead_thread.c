/*
 * overhead_thread.c
 *
 * Creates N threads. Each thread just prints a message and exits;
 * the main program times how long it takes to create and join all of them.
 *
 * Usage: ./overhead_thread <num_threads>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Function for tracking time
static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// The work each thread will do
static void *worker(void *arg) {
    (void)arg;
    printf("thread created\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    // Check the arguments
    if (argc != 2) {
        fprintf(stderr, "usage: %s <num_threads>\n", argv[0]);
        exit(1);
    }
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "num_threads must be a positive integer\n");
        exit(1);
    }

    // Allocate an array to store the threads
    pthread_t *threads = malloc((size_t)n * sizeof(pthread_t));
    if (threads == NULL) {
        perror("malloc");
        exit(1);
    }

    double start = now();

    // Create the threads
    for (int i = 0; i < n; i++) {
        int rc = pthread_create(&threads[i], NULL, worker, NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    double end = now();
    free(threads);

    double elapsed = end - start;
    printf("Created and joined %d threads in %.6f ms (%.2f ms/thread)\n",
           n, elapsed*1000.0, elapsed*1000.0/n);

    return 0;
}
