#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "udp.h"
#include "utils.h"

void initUDPclientStructure(struct addrinfo *_hints) {
	memset(_hints, 0 ,sizeof(*_hints));
    _hints->ai_family = AF_INET;    	//IPv4
    _hints->ai_socktype = SOCK_DGRAM;  //UPD Socket
    _hints->ai_flags = AI_PASSIVE|AI_NUMERICSERV;
}

struct addrinfo * createUPDsocket(int *fd, char _ip[], char _port[]) {
	struct addrinfo hints, *res;

	initUDPclientStructure(&hints);

	int n = getaddrinfo(_ip, _port, &hints, &res);
    if(n != 0) {
        printf("Error on getaddrinfo\n");
        exit(1);
    }

    *fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(*fd == -1) {
        printf("Error on socket creation\n");
        exit(1);
    }

    return res;
}

void initUDPserverStructure(struct addrinfo *_hints) {
	memset(_hints, 0 ,sizeof(*_hints));
    _hints->ai_family = AF_INET;    	//IPv4
    _hints->ai_socktype = SOCK_DGRAM;  //UPD Socket
    _hints->ai_flags = AI_NUMERICSERV;
}

int initUDPserver() {
	int fd;
	struct addrinfo hints, *res;

	initUDPserverStructure(&hints);

	int n = getaddrinfo(NULL, uport, &hints, &res);
    if(n!=0) {
        /*error*/
        exit(1);
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fd==-1) {
        /*error*/
        exit(1);
    }

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if(n == -1) {
        printf("Erro binding\n");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        exit(1);
    }

	return fd;
}


int sendUdp(int _fd, char data[], int size, struct addrinfo *_res) {
	int n;
	n = sendto(_fd, data, size, 0 , _res->ai_addr, _res->ai_addrlen);
	if (n == -1) {
		perror("Error sending msg via UDP. \n");
		exit(1);
	}

	return n;
}

int answerUdp(int _fd, char data[], int size, struct  sockaddr * _addr) {    
    int n = sendto(_fd, data, size, 0, _addr, sizeof(*_addr));
    if(n == -1){
        printf("error sending To in answerUp");
        exit(1);
    }

    return n;
}

int checkReadUdp(int _fd, struct sockaddr_in *_addr) {
    socklen_t addrlen = sizeof(*_addr);
    char buffer[PACKAGE_TCP];

    int flags = MSG_PEEK;

    // Does a MSG_PEEK to check how many bits has to read
    int nToRead = recvfrom(_fd, buffer, PACKAGE_TCP, flags, (struct sockaddr *) _addr, &addrlen);
    if(nToRead == -1) {
        printf("Error in receive from UDP \n");
        exit(1);
    }

    return nToRead;
}

int receiveUdp(int _fd, char buffer[], int size, struct sockaddr_in *_addr) {
	socklen_t addrlen = sizeof(*_addr);

	int n = recvfrom(_fd, buffer, size, 0, (struct sockaddr *) _addr, &addrlen);
    if(n == -1) {
        printf("Error in receive from UDP \n");
        exit(1);
    }
    buffer[n] = '\0';

    return n;
}

