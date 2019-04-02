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
#include "list.h"
#include "tcp.h"
#include "udp.h"

/*
 POPREQ
 sends the message to clients on the tree
 return
 -1 - error
 otherwise number of bytes sent
 */
int POPREQ(int _fd, struct addrinfo *res) {
	int n = 0;

	if(sendUdp(_fd, "POPREQ\n", strlen("POPREQ\n"), res) != strlen("POPREQ\n")) {
		return -1;
	}
	return n;
}

int POPRESP(int _fd, struct sockaddr_in *_addr, char _ipaddr[], char _tport[]) {
	char buffer[BUFFER_SIZE];
	// Creates WHOISROOT message
    strcpy(buffer, "POPRESP ");
    strcat(buffer, streamId);
    strcat(buffer, " ");
    strcat(buffer, _ipaddr);
    strcat(buffer, ":");
    strcat(buffer, _tport);
    strcat(buffer, "\n");

    // finds the size of the WHOISROOT message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    answerUdp(_fd, buffer, i + 1, (struct sockaddr *) _addr);
	
	return 1;
}

