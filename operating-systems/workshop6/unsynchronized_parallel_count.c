/*
 * unsynchronized_parallel_count.c
 *
 * Each of num_threads threads increments a shared, unprotected counter
 * iterations_per_thread times. Since "count = count + 1" is really a
 * load/add/store sequence, threads can interleave and lose updates.
 *
 * Usage: ./unsynchronized_parallel_count <num_threads> <iterations_per_thread>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

static volatile long count = 0;
static long iterations_per_thread;

// Function for tracking time
static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// The work each thread will do
static void *worker(void *arg) {
    (void)arg;
    for (long i = 0; i < iterations_per_thread; i++) {
        count = count + 1;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Check the arguments
    if (argc != 3) {
        fprintf(stderr, "usage: %s <num_threads> <iterations_per_thread>\n", argv[0]);
        exit(1);
    }
    int num_threads = atoi(argv[1]);
    iterations_per_thread = atol(argv[2]);
    if (num_threads <= 0 || iterations_per_thread <= 0) {
        fprintf(stderr, "num_threads and iterations_per_thread must be positive\n");
        exit(1);
    }

    // Allocate an array to store the threads
    pthread_t *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        perror("malloc");
        exit(1);
    }

    double start = now();

    // Create the threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    // Wait for them to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end = now();
    free(threads);

    long expected = (long)num_threads * iterations_per_thread;
    double elapsed = end - start;

    printf("Final count:    %ld\n", count);
    printf("Expected count: %ld\n", expected);

    if (count != expected) {
        printf("%ld Data races detected\n", expected - count);
    } else {
        printf("No data races detected\n");
    }

    printf("Performance: %.2fM increments/ms (%ld total increments in %.6f ms)\n",
           (expected/1000000.0) / (elapsed*1000.0), expected, elapsed*1000.0);

    return 0;
}
