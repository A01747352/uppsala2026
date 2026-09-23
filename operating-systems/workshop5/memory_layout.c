#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STACK1_SIZE (1*1024*1024)
#define STACK2_SIZE (2*1024*1024)
#define STACK3_SIZE (3*1024*1024)

#define HEAP_A_SIZE (2*1024*1024)
#define HEAP_B_SIZE (4*1024*1024)
#define HEAP_C_SIZE (5*1024*1024)

#define HEAP_FITS_HOLE_SIZE     (3*1024*1024)
#define HEAP_TOO_BIG_SIZE      (5*1024*1024)

static int global_initialized = 123;
static int global_uninitialized;

static void pause_here(const char *message)
{
    printf("\n--- %s ---\n", message);
    printf("PID = %d\n", getpid());
    printf("Press ENTER to continue...\n");
    fflush(stdout);
    getchar();
}

/*
 * Fill every byte of a memory region.
 *
 * This makes sure that all pages containing the allocation are
 * actually touched by the process.
 */
static void fill_memory(char *memory, size_t size, char value)
{
    for (size_t i = 0; i < size; i++) {
        memory[i] = value;
    }
}


int main(void)
{
    printf("PID: %d\n", getpid());

    printf("\nProgram/static data:\n");
    printf("  main()                 %p\n", (void *)main);
    printf("  global_initialized     %p\n", (void *)&global_initialized);
    printf("  global_uninitialized   %p\n", (void *)&global_uninitialized);

    pause_here("program started");

    /*
     * Stack allocations.
     *
     * Change the #define values above to make these larger or smaller.
     */
    char stack1[STACK1_SIZE];
    char stack2[STACK2_SIZE];
    char stack3[STACK3_SIZE];

    /*
     * Actually write to every byte of each stack allocation.
     */
    fill_memory(stack1, sizeof(stack1), 1);
    fill_memory(stack2, sizeof(stack2), 2);
    fill_memory(stack3, sizeof(stack3), 3);

    printf("\nStack objects:\n");
    printf("  stack1: %p - %p   (%d bytes)\n",
           (void *)stack1,
           (void *)(stack1 + sizeof(stack1)),
           STACK1_SIZE);

    printf("  stack2: %p - %p   (%d bytes)\n",
           (void *)stack2,
           (void *)(stack2 + sizeof(stack2)),
           STACK2_SIZE);

    printf("  stack3: %p - %p   (%d bytes)\n",
           (void *)stack3,
           (void *)(stack3 + sizeof(stack3)),
           STACK3_SIZE);

    pause_here("after stack allocations");

    /*
     * Initial heap allocations.
     */
    char *a = malloc(HEAP_A_SIZE);
    char *b = malloc(HEAP_B_SIZE);
    char *c = malloc(HEAP_C_SIZE);

    if (!a || !b || !c) {
        perror("malloc");
        return 1;
    }

    printf("\nInitial heap allocations:\n");
    printf("  a: %p - %p   (%d bytes)\n",
           (void *)a, (void *)(a + HEAP_A_SIZE), HEAP_A_SIZE);

    printf("  b: %p - %p   (%d bytes)\n",
           (void *)b, (void *)(b + HEAP_B_SIZE), HEAP_B_SIZE);

    printf("  c: %p - %p   (%d bytes)\n",
           (void *)c, (void *)(c + HEAP_C_SIZE), HEAP_C_SIZE);

    pause_here("after initial heap allocations");


    printf("\nWriting to the heap data:\n");
    /*
     * Again, write to every byte.
     */
    fill_memory(a, HEAP_A_SIZE, 10);
    fill_memory(b, HEAP_B_SIZE, 20);
    fill_memory(c, HEAP_C_SIZE, 30);

    pause_here("after writing to the heap allocations");

    /*
     * Free the middle allocation.
     *
     * Conceptually this leaves a hole between a and c.
     */
    printf("\nFreeing b at %p (%d bytes)\n",
           (void *)b, HEAP_B_SIZE);

    free(b);
    b = NULL;

    pause_here("after freeing b: there is now a heap hole");

    /*
     * This allocation is smaller than the old b allocation.
     *
     * A malloc implementation will often reuse the freed block.
     */
    char *d = malloc(HEAP_FITS_HOLE_SIZE);

    if (!d) {
        perror("malloc");
        return 1;
    }

    fill_memory(d, HEAP_FITS_HOLE_SIZE, 4);

    printf("\nAllocation that fits in old b's hole:\n");
    printf("  d: %p - %p   (%d bytes)\n",
           (void *)d,
           (void *)(d + HEAP_FITS_HOLE_SIZE),
           HEAP_FITS_HOLE_SIZE);

    printf("  old b started at the address printed earlier.\n");

    pause_here("after allocation that fits in the hole");

    /*
     * This request is bigger than the old b allocation, so it cannot
     * fit entirely in that original hole.
     */
    char *e = malloc(HEAP_TOO_BIG_SIZE);

    if (!e) {
        perror("malloc");
        return 1;
    }

    fill_memory(e, HEAP_TOO_BIG_SIZE, 10);

    printf("\nAllocation too large for old b's hole:\n");
    printf("  e: %p - %p   (%d bytes)\n",
           (void *)e,
           (void *)(e + HEAP_TOO_BIG_SIZE),
           HEAP_TOO_BIG_SIZE);

    pause_here("after allocation too large for the hole");

    free(a);
    free(c);
    free(d);
    free(e);

    return 0;
}