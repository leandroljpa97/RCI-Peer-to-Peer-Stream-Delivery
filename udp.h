#ifndef UDP_H_INCLUDE
#define UDP_H_INCLUDE

void initUdp(struct addrinfo *_hints);

void initUdpServer(struct addrinfo *_hints);

struct addrinfo * createUdpSocket(int *_fdUdp, char ip[], char port[], struct addrinfo * _hints_udp);

int createUpdAccessServer(char port[], struct addrinfo *_hints);

int sendUdp(int _fd, char data[], int size, struct addrinfo *_res);

int receiveUdp(int _fd, char buffer[], int size, struct sockaddr_in *_addr);

#endif