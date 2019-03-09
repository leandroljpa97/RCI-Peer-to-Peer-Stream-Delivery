#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "utils.h"



// Defualt init variables
char streamId[64];
char streamName[44];
char streamIp[IP_SIZE];
char streamPort[] = DEFAULT_STREAM_PORT;
char ipaddr[IP_SIZE];
char tport[] = DEFAULT_TPORT;
char uport[] = DEFAULT_UPORT;
char rsaddr[] = DEFAULT_RSADDR;
char rsport[] = DEFAULT_RSPORT;
int tcpsessions = DEFAULT_TCP_SESSIONS;
int bestpops = DEFAULT_BEST_POPS;
int tsecs = DEFAULT_TSECS;
int dataStream = DEFAULT_DATA_STREAM;
int debug = DEFAULT_DEBUG;


void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd) {
    FD_ZERO(_fd_sockets);
    FD_SET(0,_fd_sockets);
    _maxfd = 0;
}

/*
 * addFd: add a new file descriptor to be controlled by select
 */
void addFd(fd_set * _fd_sockets, int* _maxfd, int _fd) {
    FD_SET(_fd, _fd_sockets);
    if(*_maxfd < _fd)
        *_maxfd = _fd;
}
        

int checkPort(int _port){
    if(_port>1024 && _port <65535)
        return 1;
    return 0;
}

int insertFdClient(int _newfd, clients_t *_clients) {
    for(int i = 0; i < tcpsessions; i++){
        if(_clients->fd[i] == 0){
            _clients->fd[i] = _newfd;
            return 1;
        }
    }
    return 0;
}

int deleteFdClient(int _delfd, clients_t *_clients) {
    for(int i = 0; i < tcpsessions; i++){
        if(_clients->fd[i] == _delfd){
            _clients->fd[i] = 0;
            _clients->available++;
            return 1;
        }
    }
    return 0;
}


int readInputArguments(int argc, const char* argv[], char streamId[], char streamName[],
    					char streamIP[], char streamPort[], char ipaddr[], 
						char  tport[], char uport[], char rsaddr[], char rsport[],
						int * tcpsessions, int * bestpops, int * tsecs, 
						int *dataStream, int *debug) {

    //flag to check if there is streamId in input parameters
    int flag_streamId = 0;
    if(argc < 2) {
        // Show available Streams
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {

        if(strcmp(argv[i], "-i") == 0) {
            i++;
            if(sscanf(argv[i], "%s", ipaddr) != 1) {
                printf("Error decoding ipaddr\n");
                exit(1);
            }
            printf("ipaddr %s\n", ipaddr);
        }
        else if(strcmp(argv[i], "-t") == 0) {
            i++;
            if(sscanf(argv[i], "%s", tport) != 1) {
                printf("Error decoding tport\n");
                exit(1);
            }
            printf("tport %s\n", tport);
            if(!checkPort(atoi(tport)))
            {
                printf("port not available \n");
                exit(1);
             }       
        }
        else if(strcmp(argv[i], "-u") == 0) {
            i++;
            if(sscanf(argv[i], "%s", uport) != 1) {
                printf("Error decoding uport\n");
                exit(1);
            }
            printf("uport %s\n", uport);
            if(!checkPort(atoi(uport)))
            {
                 printf("port not available \n");
                 exit(1);
            }
        }
        else if(strcmp(argv[i], "-s") == 0) {
            i++;
            if(sscanf(argv[i], "%[^:]:%s", rsaddr, rsport) != 2) {
                printf("Error decoding rsaddr, rsport\n");
                exit(1);
            }
            printf("rsaddr %s, rsport %s\n", rsaddr, rsport);
            if(!checkPort(atoi(rsport))) {
                 printf("port not available \n");
                 exit(1);
            }
        }
        else if(strcmp(argv[i], "-p") == 0) {
            i++;
            if(sscanf(argv[i], "%d", tcpsessions) != 1) {
                printf("Error decoding tcpsessions\n");
                exit(1);
            }
            if((*tcpsessions)<1) {
                printf("tcpsessions has to be greater than 1 \n");
                exit(1);
            }
            printf("tcpsessions %d\n", *tcpsessions);
        }
        else if(strcmp(argv[i], "-n") == 0) {
            i++;
            if(sscanf(argv[i], "%d", bestpops) != 1) {
                printf("Error decoding bestpops\n");
                exit(1);
            }
            printf("bestpops %d\n", *bestpops);
             if(*(bestpops)<1){
                printf("bestpops has to be greater than 1 \n");
                exit(1);
            }
        }
        else if(strcmp(argv[i], "-x") == 0) {
            i++;
            if(sscanf(argv[i], "%d", tsecs) != 1) {
                printf("Error decoding tsecs\n");
                exit(1);
            }
            printf("tsecs %d\n", *tsecs);
        }
        else if(strcmp(argv[i], "-b") == 0) {
            *dataStream = 0;
            printf("Stream Data Activated\n");
        }
        else if(strcmp(argv[i], "-d") == 0) {
            *debug = 1;
            printf("Debug Mode Activated\n");
        }
        else if(strcmp(argv[i], "-h") == 0) {
            printf("Command line commands:\n");
            exit(0);
        }
        else {
            if(sscanf(argv[i], "%s", streamId) != 1) {
                printf("error in streamId \n");
                exit(1);
            }
            if(strlen(streamId) > 63) {
                 printf("the length of streamId is greater then 63 charact \n");
                 return 1;
            }
            if(sscanf(streamId,"%[^:]:%[^:]:%s", streamName, streamIP, streamPort)!=3){
                printf("error in streamId parameters \n");
                return 1;
            }
            // A porta que é ali indicada é a porta do servidor de stream, nao da nossa maquina logo aqui nao sabes se esta bem ou nao
            if(!checkPort(atoi(streamPort))){
                printf("Port not available \n");
                return 1;
            }

            flag_streamId = 1;
        }
    }

    if(flag_streamId)
        return 0;

    //in the case that the user doesn't input streamId, return 1 to do dump
    return 1;

}

void setTimeOut(struct timeval*_t1, struct timeval *_t2) {
    _t1 = NULL;
    _t2->tv_usec = 0;
    _t2->tv_sec = TIMEOUT;
    _t1 = _t2;
}


