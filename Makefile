CC=gcc
CFLAGS=-std=c90 -Wall -Wextra

.PHONY: clean
wzc: wzc.c
	${CC} ${CFLAGS} $^ -o $@
test: wzc
	./wzc test.c
	${CC} ${CFLAGS} out.s
clean:
	rm -f wzc a.out out.s
