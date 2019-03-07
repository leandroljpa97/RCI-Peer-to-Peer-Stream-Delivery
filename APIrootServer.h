#ifndef API_ROOTSERVER_H_INCLUDE
#define API_ROOTSERVER_H_INCLUDE

int whoIsRoot(char _rsaddr[], char _rsport[], char _streamId[], char _streamIp[], 
              char _streamPort[], char _ipaddr[], char _uport[], char _ipaddrRootStream[], 
              char _uportRootStream[], int *state, int *root, struct addrinfo *hints_accessServer, 
              struct addrinfo *hints_tcp, struct addrinfo *res_tcp, int *fdAccessServer, int *fdUp) ;

void Remove(char _streamId[], char _rsaddr[], char _rsport[]);

void dump(char _rsaddr[], char _rsport[]);

#endif