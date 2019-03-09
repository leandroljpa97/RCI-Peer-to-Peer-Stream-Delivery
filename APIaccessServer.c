#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "APIaccessServer.h"
#include "utils.h"
#include "tcp.h"


/*
 POPREQ
 sends the message to all clients on the tree
 return
 -1 - sucess
 otherwise the file descriptor of the client that didn't respond
 */
int POPREQ(clients_t clients) {

	return -1;
}

int POPRESP(int _fdUP) {

	return 1;
}