#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "tcp.h"

void initTcp(struct addrinfo *hints_tcp){
    memset(hints_tcp, 0 ,sizeof(*hints_tcp));
    hints_tcp->ai_family=AF_INET;    
    hints_tcp->ai_socktype=SOCK_STREAM;   
    hints_tcp->ai_flags= AI_NUMERICSERV;
}



int readTcp(int _fd, char* buffer, int size)
{
    int n = read(_fd, buffer, size);
    if(n == -1){
        printf("error reading in TCP \n");
        exit(1);
    }
    
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