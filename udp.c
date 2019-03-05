#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "udp.h"

#define BUFFSIZE 128


void initUdp(struct addrinfo *hints){
	memset(hints, 0 ,sizeof(*hints));
    hints->ai_family=AF_INET;    //IPv4
    hints->ai_socktype=SOCK_DGRAM;   //UPD Socket
    hints->ai_flags= AI_NUMERICSERV;
}

struct addrinfo * createUdpSocket(int *_fdUdp, char ip[], char port[], struct addrinfo * _hints_udp) {
	struct addrinfo * res;

	int n = getaddrinfo(ip, port, _hints_udp, &res);
   	if(n != 0) {
        printf("Error in getaddrinfo from ROOT_SERVER \n");
        exit(1);
    }

    *_fdUdp = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(*_fdUdp == -1) {
        printf("Error creating socket UDP to root_server \n");
        exit(1);
    }

    return res;
}

int sendUdp(int _fd, char data[], int size, struct addrinfo *_res ){
	int n;
	n = sendto(_fd, data, size, 0 , _res->ai_addr, _res->ai_addrlen);
	if (n == -1) {
		perror("Error sending msg via UDP. \n");
		exit(1);
	}

	return n;
}

int receiveUdp(int _fd, char buffer[], int size, struct sockaddr_in *_addr){
	socklen_t addrlen = sizeof(* _addr);

	int n = recvfrom(_fd, buffer, size, 0, (struct sockaddr *) _addr, &addrlen);
    if(n == -1) {
        printf("Error in receive from UDP \n");
        exit(1);
    }

    return n;
}

