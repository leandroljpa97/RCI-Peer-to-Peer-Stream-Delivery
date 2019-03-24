#ifndef API_ROOTSERVER_H_INCLUDE
#define API_ROOTSERVER_H_INCLUDE

void ctrl_c_callback_handler(int signum);

void initializations();

void DadLeft(int * _root, int * _fdAccessServer, int * _fdUp);

int WHOISROOT(int *root, int *fdAccessServer, int *fdUp);

int REMOVE();

int DUMP();



#endif