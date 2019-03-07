#ifndef TCP_H_INCLUDE
#define TCP_H_INCLUDE


void initTcp(struct addrinfo *hints_tcp);

int connectToTcp(char streamIP[], char streamPort[], struct addrinfo *hints_tcp, struct addrinfo **res_tcp);

int readTcp(int _fd, char* buffer, int size);

void writeTcp(int _fd, char *data, int size);

#endif
