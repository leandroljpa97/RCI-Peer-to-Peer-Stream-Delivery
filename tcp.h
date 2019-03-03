#ifndef TCP_H_INCLUDE
#define TCP_H_INCLUDE


void initTcp(struct addrinfo *hints_tcp);
void readTcp(int fd, char* buffer);
void write_tcp(int fd, char *msg);

#endif
