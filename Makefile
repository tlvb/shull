CC=gcc
RM=rm
CFLAGS=-g -Wall -DLL_CIRCULAR -D_GNU_SOURCE -lm
srcs=shull.c ll.c
objs=$(patsubst %.c,%.o,$(srcs))

test: test_main.c shull.o ll.o
	$(CC) $(CFLAGS) -o $@ $^
	test -d data || mkdir data

shull.o: shull.c shull.h ll.h
	$(CC) $(CFLAGS) -c $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) test $(objs) || true

ll.c ll.h:
	test -d linked_list || git clone https://github.com/tlvb/linked_list
	test -f ll.c || ln -s linked_list/ll.c
	test -f ll.h || ln -s linked_list/ll.h
