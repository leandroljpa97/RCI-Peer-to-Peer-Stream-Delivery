#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "tcp.h"
#include "utils.h"

void initTcp(struct addrinfo *_hints){
    memset(_hints, 0 ,sizeof(*_hints));
    _hints->ai_family=AF_INET;    
    _hints->ai_socktype=SOCK_STREAM;   
    _hints->ai_flags= AI_NUMERICHOST | AI_NUMERICSERV;
}


void initTcpServer(struct addrinfo *_hints){
    memset(_hints, 0 ,sizeof(*_hints));
    _hints->ai_family=AF_INET;    
    _hints->ai_socktype=SOCK_STREAM;   
    _hints->ai_flags= AI_PASSIVE | AI_NUMERICSERV;
}

/*

 return:
        0 = ERROR
        fdUp = file descriptor to communicate with adjacent iamroot
*/
int connectToStream() {
    
	struct addrinfo hints, *res_tcp;

	initTcp(&hints);

    int n = getaddrinfo(streamIp, streamPort, &hints, &res_tcp);
    if(n != 0) {
        printf("error getaddrinfo in TCP source server \n");
        exit(1);
    }

    int fdUp = socket(res_tcp->ai_family, res_tcp->ai_socktype, res_tcp->ai_protocol);
    if(fdUp == -1) {
        printf("error creating TCP socket TCP to source server!! \n ");
        exit(1);
    }

    n = connect(fdUp, res_tcp->ai_addr, res_tcp->ai_addrlen);
    if(n == -1) {
        printf("error in connect with TCP socket TCP in source!! \n ");
        exit(1);
    }

    return fdUp;
}

int connectToTcp(char _availableIAmRootIP[], char _availableIAmRootPort[]) {
    struct addrinfo hints, *res_tcp;

    initTcp(&hints);

    int n = getaddrinfo(_availableIAmRootIP, _availableIAmRootPort, &hints, &res_tcp);
    if(n != 0) {
        if(status == DAD_LOST)
            return -1;
        printf("error getaddrinfo in TCP source server  2\n");
        exit(1);
     }

    int fdUp = socket((res_tcp)->ai_family, (res_tcp)->ai_socktype, (res_tcp)->ai_protocol);
    if(fdUp == -1 ) {
        if(status == DAD_LOST)
            return -1;
        printf("error creating TCP socket TCP to source server...2. \n ");
        exit(1);
    }

    n = connect(fdUp, (res_tcp)->ai_addr, (res_tcp)->ai_addrlen);
    if(n == -1 ) {
        if(status == DAD_LOST)
            return -1;
        printf("error in connect with TCP socket TCP in source.... 2 \n ");
        exit(1);
    }

    printf("o fd depois do tcp é : %d \n", fdUp);

    return fdUp;
}

int createTcpServer() {
    struct  addrinfo hints, *res;

    initTcpServer(&hints);

    int n = getaddrinfo(NULL, tport, &hints, &res);
    if(n != 0) {
        printf("Error getting addr info TCP server\n");
        exit(1);
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fd == -1) {
        printf("Error creating socket\n");
        exit(1);
    }

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if(n == -1) {
        printf("Error in bind on TCP Server\n");
        exit(1);
    }

    if(listen(fd, tcpsessions) == -1) {
        printf("Error listen\n");
        exit(1);
    }

    printf("waiting new clients \n");

    return fd;
}



int readTcp(int fd, char* buffer) {
    int nReceived;   
    char aux[PACKAGETCP];    

    nReceived = read(fd, aux, sizeof(aux));
    if(nReceived <= 0){
        printf("error receiving message \n");
        return 0;
    }

    aux[nReceived] = '\0';
    strcat(buffer, aux);

    return (int) strlen(buffer);
}

int readTcpNBytes(int fd, char* buffer, int nbytes) {
    int nReceived;   
    char aux[PACKAGETCP];    

    nReceived = read(fd, aux, nbytes);
    if(nReceived <= 0){
        printf("error receiving message \n");
        return 0;
    }

    aux[nReceived] = '\0';
    strcat(buffer, aux);

    return nReceived;
}



int writeTcp(int _fd, char *data, int size) {
    printf("dentro do tcp é size : %d \n",size);
    int nSended;   
    int nbytes;     
    int nLeft;      

    nbytes = size; 
    nLeft = nbytes;

    while(nLeft > 0) {
        nSended = write(_fd, data, nLeft);
        if(nSended < 0){
            printf("error sending message \n");
            return -1;
        }

        nLeft -= nSended;
        data += nSended;
    }

    return nbytes;
}