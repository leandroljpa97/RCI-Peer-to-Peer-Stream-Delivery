#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

// Library for special types of Int
#include <stdint.h>

#define TRIES 3
#define TIMEOUT 4
#define TIMEOUT_REMOVE 2

#define BUFFER_SIZE 256
#define PACKAGETCP 256

#define IP_SIZE 16
#define PORT_SIZE 6

#define DEFAULT_STREAM_PORT "00000"
#define DEFAULT_TPORT "59000"
#define DEFAULT_UPORT "59000"
#define DEFAULT_RSADDR "193.136.138.142"
#define DEFAULT_RSPORT "59000"
#define DEFAULT_TCP_SESSIONS 1
#define DEFAULT_BEST_POPS 1
#define DEFAULT_TSECS 5
#define DEFAULT_DATA_STREAM 1
#define DEFAULT_DEBUG 0

extern char streamId[];
extern char streamIp[];
extern char streamName[];
extern char streamPort[];
extern char ipaddr[];
extern char tport[];
extern char uport[];
extern char rsaddr[];
extern char rsport[];
extern int tcpsessions;
extern int bestpops;
extern int tsecs;
extern int dataStream;
extern int debug;

typedef struct _clients {
	int *fd;
	int available;
} clients_t;


void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd);

void addFd(fd_set * _fd_sockets, int* _maxfd, int _fd);

int insertFdClient(int _newfd, clients_t *_clients);

int deleteFdClient(int _delfd, clients_t *_clients);

int checkPort(int _port);

int readInputArguments(int argc, const char* argv[], char streamId[], char streamName[],
    					char streamIP[], char streamPort[], char ipaddr[], 
						char  tport[], char uport[], char rsaddr[], char rsport[],
						int * tcpsessions, int * bestpops, int * tsecs, 
						int *dataStream, int *debug);

void setTimeOut(struct timeval*_t1, struct timeval *_t2);


#endif