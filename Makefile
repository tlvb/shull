CC=gcc
RM=rm
CFLAGS=-g -Wall -DLL_CIRCULAR -D_GNU_SOURCE -lm
srcs=shull.c ll.c
objs=$(patsubst %.c,%.o,$(srcs))

test: test_main.c shull.o ll.o
	$(CC) $(CFLAGS) -o $@ $^
	test -d data || mkdir data

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) test $(objs) || true

