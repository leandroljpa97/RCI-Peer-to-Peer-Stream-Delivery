#ifndef UDP_H_INCLUDE
#define UDP_H_INCLUDE

void initUDPclientStructure(struct addrinfo *hints);

struct addrinfo * createUPDsocket(int *fd, char _ip[], char _port[]);

void initUDPserverStructure(struct addrinfo *_hints);

int initUDPserver();

int sendUdp(int _fd, char data[], int size, struct addrinfo *_res);

int answerUdp(int _fd, char data[], int size, struct  sockaddr * _addr);

int checkReadUdp(int _fd, struct sockaddr_in *_addr);

int receiveUdp(int _fd, char buffer[], int size, struct sockaddr_in *_addr);

#endif