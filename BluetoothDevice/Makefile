ifndef CC
	CC=gcc
endif
CFLAGS=-std=c99 -Wall -pedantic -Wextra
SRCS=main.c
OBJS=$(subst .c,.o,$(SRCS))
RM=rm -f

all: main

foo: main.o
	$(CC) $(CFLAGS) -o main main.c

clean:
	$(RM) $(OBJS) main

