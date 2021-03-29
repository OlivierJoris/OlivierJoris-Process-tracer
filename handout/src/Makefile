CC=gcc 
CFLAGS=-g --pedantic -Wall -Wextra -Wmissing-prototypes -std=c99
PROGRAM=tracer 
# DO NOT MODIFY CC, CFLAGS and PROGRAM
# COMPLETE BELOW

SOURCES=file_sys_calls.c functions_addresses.c profiler.c syscall.c load_param.c tracer.c
OBJECTS=$(SOURCES:.c=.o)

all: $(PROGRAM)

tracer: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm $(PROGRAM)
	clear