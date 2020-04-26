CC=gcc
CFLAGS=-std=c90 -Wall -Wextra

.PHONY: clean all
all: wzc
wzc: wzc.o
	${CC} ${CFLAGS} $^ -o $@
%.o: %.c
	${CC} ${CFLAGS} -c $^ -o $@
test: wzc
	./wzc test.c
	${CC} ${CFLAGS} out.s
clean:
	rm -f wzc a.out out.s *.o
