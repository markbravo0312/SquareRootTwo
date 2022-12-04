# Optimize, turn on additional warnings
CFLAGS=-std=c17 -Wall -Wextra -no-pie -pthread

DEBUG_FLAGS=-g -O0
PROFILE_FLAGS=-O3 -fno-inline-small-functions
RELEASE_FLAGS=-O3

LINKERFLAGS=-lm

.PHONY: all
all: main
main: *.c
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o $@ $^ $(LINKERFLAGS)

debug: *.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $@ $^ $(LINKERFLAGS)

prof: *.c
	$(CC) $(CFLAGS) $(PROFILE_FLAGS) -o $@ $^ $(LINKERFLAGS)


asan: *.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -fsanitize=address -o $@ $^ $(LINKERFLAGS)

.PHONY: clean
clean:
	rm -f main debug asan prof
