#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

// Simple routine to return absolute time (in seconds).
double Time_GetSeconds() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) ((double)t.tv_sec + (double)t.tv_usec / 1e6);
}

static void pause_here(const char *message)
{
    printf("\n--- %s ---\n", message);
    printf("PID = %d\n", getpid());
    printf("Press ENTER to continue...\n");
    fflush(stdout);
    getchar();
}

// Program that allocates an array of ints of certain size,
// and then proceeeds to update each int in a loop, forever.
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: spin <memory (MB)>\n");
        exit(1);
    }
    long long int size = (long long int) atoi(argv[1]);
    long long int size_in_bytes = size * 1024 * 1024;

    printf("allocating %lld bytes (%.2f MB)\n", 
	   size_in_bytes, size_in_bytes / (1024 * 1024.0));

    // the big memory allocation happens here
    int *x = malloc(size_in_bytes);
    if (x == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        exit(1);
    }

    long long int num_ints = size_in_bytes / sizeof(int);
    printf("  number of integers in array: %lld\n", num_ints);

    pause_here(" malloc() finished ");

    // now the main loop: each time through, touch each integer
    // (and increment its value by 1).
    long long int i = 0;
    double time_since_last_print = 2.0; 
    double t = Time_GetSeconds();
    int loop_count = 0;
    unsigned rand = 123456789;
    while (1) {
        // Choose one or the other:
        // Generate a psuedo random number we can use
        rand = rand * 1664525u + 1013904223u;
        // Write value + 1 to each location in memory.
        x[i++] += 1; 
        // Write a psuedo random number to each location in memory.
        // x[i++] += rand;

        // if we've gone through the whole loop, reset a bunch of stuff
        // and then (perhaps) print out some statistics. 
        if (i == num_ints) {
            double delta_time = Time_GetSeconds() - t;
            time_since_last_print += delta_time;
            if (time_since_last_print >= 0.2) { // only print every .2 seconds
            printf("loop %d in %.2f ms (bandwidth: %.2f MB/s)\n", 
                loop_count, 1000 * delta_time, 
                size_in_bytes / (1024.0*1024.0*delta_time));
            time_since_last_print = 0;
            }

            i = 0;
            t = Time_GetSeconds();
            loop_count++;
        }
    }

    return 0*rand;
}

