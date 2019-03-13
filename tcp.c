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
        printf("error getaddrinfo in TCP source server \n");
        exit(1);
     }

    int fdUp = socket((res_tcp)->ai_family, (res_tcp)->ai_socktype, (res_tcp)->ai_protocol);
    if(fdUp == -1) {
        printf("error creating TCP socket TCP to source server...2. \n ");
        exit(1);
    }

    n = connect(fdUp, (res_tcp)->ai_addr, (res_tcp)->ai_addrlen);
    if(n == -1) {
        printf("error in connect with TCP socket TCP in source.... \n ");
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

    if(listen(fd, bestpops) == -1) {
        printf("Error listen\n");
        exit(1);
    }

    printf("waiting new clients \n");

    return fd;
}


int readTcpStream(int fd, char* buffer, int size) {
    int nReceived;   
    int nbytes;     
    int nLeft;  
    char aux[PACKAGETCP];    

    nLeft = size;
    nbytes = 0;

    while(nLeft > 0) {
        printf("ola\n");
        nReceived = read(fd, aux, nLeft);
        printf("nReceived = %d \n",nReceived);
        // Received an EOF
        if(nReceived == -1) {
            printf("Dad left\n");
            return -1;
        }
        // Nothing left to read
        else if(nReceived == 0) {
            return nbytes;
        }

        nLeft -= nReceived;
        buffer += nReceived;
        nbytes += nReceived;

        aux[nReceived] = '\0';
        printf("aux %s\n", aux);
        strcat(buffer, aux);
        printf("buffer %s\n", buffer);
    }

    return nbytes;
}

int readTcp(int fd, char* buffer, int size) {
    int nReceived;   
    int nbytes;     
    int nLeft;  
    char aux[PACKAGETCP];    

    nLeft = size;
    nbytes = 0;

    while(1) {
        nReceived = read(fd, aux, nLeft);
        if(nReceived <= 0){
            printf("error receiving message \n");
            return 0;

        }

        nLeft -= nReceived;
        buffer += nReceived;
        nbytes += nReceived;

        if(aux[nReceived-1] == '\n' )
            break;

        aux[nReceived] = '\0';
        strcat(buffer, aux);
    }

    aux[nReceived] = '\0';
    strcat(buffer, aux);

    return nbytes;
}

int writeTcp(int _fd, char *data, int size) {
    int nSended;   
    int nbytes;     
    int nLeft;      

    nbytes = size; 
    nLeft = nbytes;

    while(nLeft > 0) {
        nSended = write(_fd, data, nLeft);
        if(nSended <= 0){
            printf("error sending message \n");
            return 0;
        }

        nLeft -= nSended;
        data += nSended;
    }

    return nbytes;
}