#ifndef API_ACCESS_SERVER_H_INCLUDE
#define API_ACCESS_SERVER_H_INCLUDE

#include "utils.h"

int POPREQ(int _fd, struct addrinfo *res);

int POPRESP(int _fd, struct sockaddr_in *_addr, char _ipaddr[], char _uport[]);



#endif