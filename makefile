CC=gcc
CFLAGS= -O2 -I. -Wall -std=gnu11
DEPS = utils.h upd.h tcp.h APIpairComunication.h APIaccessServer.h APIrootServer.h iamroot.h
OBJ = utils.c udp.c tcp.c APIpairComunication.c APIaccessServer.c APIrootServer.c iamroot.c

default: iamroot

all: iamroot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

iamroot: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o iamroot 