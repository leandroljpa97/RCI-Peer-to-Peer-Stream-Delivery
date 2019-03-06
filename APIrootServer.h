#ifndef API_ROOTSERVER_H_INCLUDE
#define API_ROOTSERVER_H_INCLUDE

void whoIsRoot(int _fd, struct addrinfo *_res, char _streamId[], char _ipaddr[], char _uport[]);

void Remove(int _fd, struct addrinfo *_res, char _streamId[]);

void dump(int _fd, struct addrinfo *_res);

#endif