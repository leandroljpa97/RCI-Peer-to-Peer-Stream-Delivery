#ifndef TCP_H_INCLUDE
#define TCP_H_INCLUDE


void initTcp(struct addrinfo *_hints);

void initTcpServer(struct addrinfo *_hints);

int connectToStream();

int connectToTcp(char _availableIAmRootIP[], char _availableIAmRootPort[]);

int createTcpServer();

int readTcp(int fd, char* buffer);

int writeTcp(int _fd, char *data, int size);

#endif
