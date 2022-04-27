CC=gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2

proj2: proj2.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f proj2.o proj2