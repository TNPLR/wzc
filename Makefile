CC=gcc
CFLAGS=-std=c90 -pedantic -Wall -Wextra -ggdb

.PHONY: all clean
all: wzc
wzc: wzc.o ast.o vector.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} $^ -c -o $@
clean:
	rm -f *.o wzc out.s a.out
