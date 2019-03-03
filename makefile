CFLAGS= -O2 -Ofast -Wall -std=gnu11

default: iamroot

all: iamroot


iamroot: iamroot.c udp.c tcp.c
	gcc iamroot.c udp.c tcp.c -o iamroot $(CFLAGS)

clean:
	rm -f *.o iamroot 