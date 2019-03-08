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


/*

 return:
        0 = ERROR
        fdUp = file descriptor to communicate with adjacent iamroot
*/
int connectToTcp() {
    
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

int readTcp(int fd, char* buffer, int size) {
    char aux[128];  
    int n = read(fd,aux,sizeof(aux));
    if(n==-1){
        printf("error reading in TCP \n");
        exit(1);
    }
    

    aux[n]='\0';
    strcat(buffer, aux);

    return n;
}

void writeTcp(int _fd, char *data, int size)
{
    int nSended;   
    int nBytes;     
    int nLeft;      

    nBytes = size; 
    nLeft = nBytes;

    while(nLeft > 0)
    {
        nSended = write(_fd, data, nLeft);
        if(nSended <= 0){
            printf("error sending message \n");
        }

        nLeft -= nSended;
        data += nSended;
    }

    data -= nBytes;
}