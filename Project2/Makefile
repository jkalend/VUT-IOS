CC=gcc
CFLAGS = -g -O2 -std=c99 -Wall -Wextra -pedantic

all: proj2

proj2: proj2.o
	$(CC) $(CFLAGS) $^ -o $@

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c $^ -o $@