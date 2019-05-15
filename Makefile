CC=gcc
CFLAGS=-Wextra -O3 -fopenmp -pthread
LDLIBS=-fopenmp -latomic

juez: juez.c
	$(CC) $(CFLAGS) -o juez juez.c $(LDFLAGS)
