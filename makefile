CC=gcc
CFLAGS= -O2 -I. -Wall -std=gnu11
DEPS = inout.h udp.h tcp.h iamroot.h
OBJ = inout.c udp.c tcp.c iamroot.c

default: iamroot

all: iamroot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

iamroot: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	rm -f *.o iamroot 