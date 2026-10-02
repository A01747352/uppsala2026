CC = cc
CFLAGS = -Wall -Wextra -O2 -pthread

PROGRAMS = overhead_fork \
           overhead_thread \
           unsynchronized_parallel_count \
           broken_synchronization_parallel_count \
           correctly_synchronized_parallel_count \
           faster_synchronized_parallel_count

all: $(PROGRAMS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean
