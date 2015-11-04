# makefile
PROGRAM_NAME = trafficmgr
ARGS = snnwwens
CC=gcc
CFLAGS = -Wall -pedantic-errors
make_first: cart.h q.h q.c monitor.h monitor.c trafficmgr.c
	$(CC) $(CFLAGS) cart.h q.h q.c monitor.h monitor.c trafficmgr.c -lpthread -o trafficmgr
run:
	#$(make_first)
	$(PROGRAM_NAME) $(ARGS)