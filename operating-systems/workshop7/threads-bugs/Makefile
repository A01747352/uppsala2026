
ALL = vector-deadlock vector-global-order vector-try-wait vector-avoid-hold-and-wait vector-nolock-portable vector-nolock
COMMON = vector-header.h main-common.c main-header.h common.h common_threads.h

# Note: vector-nolock uses x86-only inline assembly and will fail to build
# on arm64 (e.g. Apple Silicon Macs); this is expected. It is listed last so
# that "make" (which stops at the first failing target) still builds every
# other program, including the portable vector-nolock-portable, first.

all: $(ALL)

clean:
	rm -f $(ALL) *~

vector-deadlock: vector-deadlock.c $(COMMON)
	gcc -o vector-deadlock vector-deadlock.c -Wall -pthread -O

vector-global-order: vector-global-order.c $(COMMON)
	gcc -o vector-global-order vector-global-order.c -Wall -pthread -O

vector-try-wait: vector-try-wait.c $(COMMON)
	gcc -o vector-try-wait vector-try-wait.c -Wall -pthread -O

vector-avoid-hold-and-wait: vector-avoid-hold-and-wait.c $(COMMON)
	gcc -o vector-avoid-hold-and-wait vector-avoid-hold-and-wait.c -Wall -pthread -O

vector-nolock: vector-nolock.c $(COMMON)
	gcc -o vector-nolock vector-nolock.c -Wall -pthread -O

vector-nolock-portable: vector-nolock-portable.c $(COMMON)
	gcc -o vector-nolock-portable vector-nolock-portable.c -Wall -pthread -O

