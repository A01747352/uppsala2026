#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define N 1000000000ULL

static double elapsed_seconds(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void)
{
    uint64_t total = 0;
    double pi = 0.0;

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    printf("PID %d starting\n", getpid());

    while (1) {
        double sum = 0.0;

        for (uint64_t i = 0; i < N; i++) {
            double x = ((double)i + 0.5) / (double)N;
            sum += 4.0 / (1.0 + x * x);
        }

        pi = sum / (double)N;
        total += N;

        clock_gettime(CLOCK_MONOTONIC, &now);
        double seconds = elapsed_seconds(start, now);

        printf("PID %d: %llu B iterations, %.3f B iter/s\n",
               getpid(),
               (unsigned long long)(total / N),
               (total / seconds) / 1e9);

        fflush(stdout);
    }
    printf("Final pi value: %g\n", pi);
}