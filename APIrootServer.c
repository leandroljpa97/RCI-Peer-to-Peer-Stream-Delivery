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

    strcpy(buffer,"WHOISROOT ");
    strcat(buffer,_streamId);
    strcat(buffer," ");
    strcat(buffer,_ipaddr);
    strcat(buffer,":");
    strcat(buffer,_uport);
    strcat(buffer,"\n");
    printf("o buffer no whoIsRoot é %s \n",buffer);
    
    sendUdp(_fd, buffer, BUFFSIZE, _res);
}

void Remove(int _fd, struct addrinfo *_res, char _streamId[]){
    char buffer[BUFFSIZE];
    strcpy(buffer,"REMOVE ");
    strcat(buffer,_streamId);
    sendUdp(_fd, buffer, BUFFSIZE, _res);
}

void dump(int _fd, struct addrinfo *_res) {
    sendUdp(_fd, "DUMP\n", strlen("DUMP\n"), _res);
    printf("sai de dentro do dump \n");
}
