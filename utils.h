#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

// Library for special types of Int
#include <stdint.h>


#define TRIES 3
#define TIMEOUT 10
#define TIMEOUT_REMOVE 2

#define BUFFER_SIZE 256
#define PACKAGETCP 65536
#define TCP_MESSAGE_TYPE 3
#define TCP_MESSAGE_SIZE 5
#define TCP_MESSAGE_STREAMID 64

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

//use this structure to childreens of each iamRoot and in the case  of being root, to accessPoints
typedef struct _clients {
	int *fd;
	int available;
	char **ip;
	char **port;
} clients_t;

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

//variables stored to iamroot that is finding new accessPoint!
extern char ipAccessPoint[IP_SIZE];
extern char portAccessPoint[PORT_SIZE];

// Structure with clients information
extern clients_t clients;

// number of AP availables on the list
extern int numberOfAP;

// Indicatse the number of the Query in 16 bits
extern uint16_t queryId ;


void ctrl_c_callback_handler(int signum);

void error_confirmation(char*s);


/* CLIENT STRUCTURE FUNCTIONS */

void initClientStructure();

void addClient(int _fd, char _ip[], char _port[]);

void deleteClient(int _fd);

int insertFdClient(int _newfd);

int deleteFdClient(int _delfd); 

void closeClient(int _fd);

void closeAllClients();

void clearClientStructure();

void initializations();

void convertNumDoHex(char *nbytesHex, int num);

void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd);

void addFd(fd_set * _fd_sockets, int* _maxfd, int _fd);

int checkPort(int _port);

int readInputArguments(int argc, const char* argv[], char streamId[], char streamName[],
    					char streamIP[], char streamPort[], char ipaddr[], 
						char  tport[], char uport[], char rsaddr[], char rsport[],
						int * tcpsessions, int * bestpops, int * tsecs, 
						int *dataStream, int *debug);

void setTimeOut(struct timeval*_t1, struct timeval *_t2);


#endif