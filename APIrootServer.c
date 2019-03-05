#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "APIrootServer.h"
#include "udp.h"

#define BUFFSIZE 200

void whoIsRoot(int _fd, struct addrinfo *_res, char _streamId[], char _ipaddr[], char _uport[]) {
    char buffer[BUFFSIZE];
    printf("_ipaddr %s\n", _ipaddr);
    strcpy(buffer,"WHOISROOT ");
    strcat(buffer,_streamId);
    strcat(buffer," ");
    strcat(buffer,_ipaddr);
    strcat(buffer,":");
    strcat(buffer,_uport);
    strcat(buffer,"\n");
    printf("o buffer no whoIsRoot é %s \n",buffer);
    
    sendUdp(_fd, buffer, BUFFSIZE, _res);
    
    printf("dentro do whoirrot o fd é: %d \n",_fd);
}