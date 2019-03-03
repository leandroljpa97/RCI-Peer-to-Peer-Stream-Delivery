#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "udp.h"


void initUdp(struct addrinfo *hints){
	memset(hints, 0 ,sizeof(*hints));
    hints->ai_family=AF_INET;    //IPv4
    hints->ai_socktype=SOCK_DGRAM;   //UPD Socket
    hints->ai_flags= AI_NUMERICSERV;
}

void createUdpSocket(int *fd_udp, char ip[], char port[], struct addrinfo *res,struct addrinfo *hints_udp){
	int n;
	n = getaddrinfo(ip, port, hints_udp, &res);
   	if(n!=0) {
        printf("Error in getaddrinfo from ROOT_SERVER \n");
        exit(1);
    }

    *fd_udp = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(*fd_udp==-1) {
        printf("Error creating socket UDP to root_server \n");
        exit(1);
    }

}

void sendUdp(int _fd, char _data[], struct addrinfo *_res ){
	int n;
	n = sendto(_fd,_data, strlen(_data), 0 , _res->ai_addr, _res->ai_addrlen);
	if (n==-1) {
		perror("Error sending msg via UDP. \n");
		exit(1);
	}
}

void receiveUdp(int _fd, char _buffer[], struct sockaddr_in *_addr){
	//meter um DEFINE PARA ESTE 128 NAO?????????????????????????????????????????????
	int n;
	socklen_t addrlen;
	addrlen =sizeof(*_addr);
	n = recvfrom(_fd, _buffer, 128, 0, (struct sockaddr *)_addr, &addrlen);
	        if(n==-1) {
	            printf("Error in receive from UDP \n");
	            exit(1);
	        }
}

