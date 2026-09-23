#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define TOTAL_DATA (100L * 1024 * 1024)   /* 100 MB total */
#define READ_SIZE  1000                   /* Change this */

static double elapsed_ns(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1e9 +
           (end.tv_nsec - start.tv_nsec);
}

int main(void)
{
    struct timespec start, end;
    long iterations = TOTAL_DATA / READ_SIZE;
    long count = 0;

    char *buffer = malloc(READ_SIZE);
    if (buffer == NULL) {
        perror("malloc");
        return 1;
    }

    int fd = open("testfile", O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /*
     * Create a file exactly READ_SIZE bytes long.
     */
    for (long i = 0; i < READ_SIZE; i++)
        buffer[i] = (char)i;

    if (write(fd, buffer, READ_SIZE) != READ_SIZE) {
        perror("write");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < iterations; i++) {

        /*
         * Uncomment ONE line.
         * Leave both commented out to measure loop overhead.
         */

        //getppid();
        pread(fd, buffer, READ_SIZE, 0);

        count++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double total = elapsed_ns(start, end);

    /*
     * Use buffer so the compiler cannot regard it as unused.
     */
    count += buffer[0];
    buffer[0] = count;

    printf("Read size:        %d bytes\n", READ_SIZE);
    printf("Total data:       %ld bytes\n", TOTAL_DATA);
    printf("System calls:     %ld\n", iterations);
    printf("Total time:       %.3f ms\n", total / 1e6);
    printf("Time/system call: %.2f ns\n", total / iterations);

    close(fd);
    unlink("testfile");
    free(buffer);

    return 0;
}

// #include <stdio.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <time.h>

// #define ITERATIONS 10000000L

// static double elapsed_ns(struct timespec start, struct timespec end)
// {
//     return (end.tv_sec - start.tv_sec) * 1e9 +
//            (end.tv_nsec - start.tv_nsec);
// }

// int main(void)
// {
//     struct timespec start, end;
//     int fd;
//     long count = 0;

//     fd = open("/dev/zero", O_RDONLY);
//     if (fd < 0) {
//         perror("open");
//         return 1;
//     }

//     clock_gettime(CLOCK_MONOTONIC, &start);

//     for (long i = 0; i < ITERATIONS; i++) {

//         /*
//          * Uncomment ONE line.
//          * Leave both commented out to measure loop overhead.
//          */

//         // getppid();
//         // char buffer[1]; read(fd, buffer, 1);

//         count++;
//     }

//     clock_gettime(CLOCK_MONOTONIC, &end);

//     double total = elapsed_ns(start, end);

//     printf("Iterations:     %ld\n", count);
//     printf("Total time:     %.3f ms\n", total / 1e6);
//     printf("Time/iteration: %.2f ns\n", total / ITERATIONS);

//     close(fd);
//     return 0;
// }