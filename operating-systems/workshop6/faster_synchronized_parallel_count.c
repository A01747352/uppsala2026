/*
 * faster_synchronized_parallel_count.c
 *
 * Each thread takes the lock once to read the shared count into
 * starting_count, does all of its increments on a private local_count
 * (no locking needed since it is not shared), then takes the lock once
 * more to add the difference (local_count - starting_count) back into
 * the shared count. This uses the lock only twice per thread instead of
 * once per increment, but the shared count is only accurate once every
 * thread has finished.
 *
 * Usage: ./faster_synchronized_parallel_count <num_threads> <iterations_per_thread>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

static long count = 0;
static long iterations_per_thread;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Function for tracking time
static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// The work each thread will do
static void *worker(void *arg) {
    (void)arg;

    // Get the count before the thread starts
    pthread_mutex_lock(&lock);
    long starting_count = count;
    pthread_mutex_unlock(&lock);

    // Do the work on a local count
    long local_count = starting_count;
    for (long i = 0; i < iterations_per_thread; i++) {
        local_count = local_count + 1;
    }

    // Update the count
    pthread_mutex_lock(&lock);
    count = count + (local_count - starting_count);
    pthread_mutex_unlock(&lock);

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
