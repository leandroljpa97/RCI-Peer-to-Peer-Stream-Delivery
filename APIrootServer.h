#ifndef API_ROOTSERVER_H_INCLUDE
#define API_ROOTSERVER_H_INCLUDE

void whoIsRoot(char _rsaddr[], char _rsport[], char _streamId[], char _ipaddr[], char _uport[], char _ipaddrRootStream[] , char _uportRootStream[]);

void Remove(char _streamId[], char _rsaddr[], char _rsport[]);

void dump(char _rsaddr[], char _rsport[]);

#endif