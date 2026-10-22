CC=gcc
CFLAGS=-I.

all: process_memory zombcreate parallel_sum sum_lib.o

process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

zombcreate : zombdemo.c
	$(CC) -o zombcreate zombdemo.c $(CFLAGS)

parallel_sum : parallel_sum.c sum_lib.o sum_lib.h
	$(CC) -pthread -o parallel_sum parallel_sum.c sum_lib.o $(CFLAGS)

sum_lib.o : sum_lib.h
	$(CC) -o sum_lib.o -c sum_lib.c $(CFLAGS)

clean :
	rm -f process_memory zombcreate parallel_sum sum_lib.o