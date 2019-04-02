#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

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

// Indicates if the iamroot app is a root of the stream
int root = 0;
// Structure with clients information
clients_t clients;

// number of AP availables on the list
int numberOfAP;

// Indicatse the number of the Query in 16 bits
uint16_t queryId = 0;

char ipAccessPoint[];
char portAccessPoint[];


int status = NORMAL;

//check if stream is broken
int broken = 0;

int ascii = 1;

char availableIAmRootIP[IP_SIZE];
char availableIAmRootPort[BUFFER_SIZE];

int timerBS = 0;

void error_confirmation(char*s) {
        printf("%s \n",s);
        exit(EXIT_FAILURE);
}

void AsciiToHex(char dataIn[], char dataOut[]) {
    int i, len;

 
    len = strlen(dataIn);
    if(dataIn[len-1]=='\n')
        dataIn[--len] = '\0';

    for(i = 0; i<len; i++){
        sprintf(dataOut+i*2, "%02X", dataIn[i]);
    }
}


void initClientStructure(){
    clients.available = tcpsessions;
    clients.fd = (int *) calloc(tcpsessions, sizeof(int));
    clients.ip = (char **) calloc(tcpsessions, sizeof(char *));
    clients.port = (char **) calloc(tcpsessions, sizeof(char *));
    clients.buffer = (char **) calloc(tcpsessions, sizeof(char *));
    for (int i = 0; i < tcpsessions; ++i) {
        clients.ip[i] = (char *) calloc(IP_SIZE, sizeof(char));
        clients.port[i] = (char *) calloc(PORT_SIZE, sizeof(char));
        clients.buffer[i] = (char *) calloc(PACKAGE_TCP, sizeof(char));
    }

}

void addClient(int _fd, char _ip[], char _port[]) {
    int i;
    for (i = 0; i < tcpsessions; ++i) {
        if(clients.fd[i] == _fd) {
            break;
        }
    }
    strcpy(clients.ip[i], _ip);
    strcpy(clients.port[i], _port);
    memset(clients.buffer[i], '\0', PACKAGE_TCP);
}




/*
 * insertFdClient: inserts a fd on a empty position and decreases the number of available positions
   return: 1 - success
           0 - no empty position found, fd not added
 */
int insertFdClient(int _newfd) {
    for(int i = 0; i < tcpsessions; i++){
        if(clients.fd[i] == 0){
            clients.fd[i] = _newfd;
            clients.available--;
            return 1;
        }
    }
    return 0;
}

/*
 * insertFdClient: deletes a fd and increases the number of available positions
   return: 1 - success
           0 - no fd position found, fd not deleted
 */
int deleteFdClient(int _delfd) {
    for(int i = 0; i < tcpsessions; i++){
        if(clients.fd[i] == _delfd){
            clients.fd[i] = 0;
            clients.available++;
            return 1;
        }
    }
    return 0;
}

int getIndexChild(int _index){
    for(int i = _index; i < tcpsessions; i++)
        if(clients.fd[i] != 0)
            return i;

    int n = getIndexChild(0);
    return n;
   
}




void clearClientStructure() {
    free(clients.fd);
    for (int i = 0; i < tcpsessions; ++i) {
        free(clients.ip[i]);
        free(clients.port[i]);
        free(clients.buffer[i]);
    }
    free(clients.ip);
    free(clients.port);
    free(clients.buffer);
}


void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd) {
    FD_ZERO(_fd_sockets);
    FD_SET(0,_fd_sockets);
    _maxfd = 0;
}

void convertNumDoHex(char *nbytesHex, int num) {
    int i = 0;

    // Converts queryID to hex format
    sprintf(nbytesHex, "%x", num);

    char hex = nbytesHex[i];
    while(hex != '\0') {
        i++;
        hex = nbytesHex[i];
    }
    i--;
    for(int j = 3; j >= 0; j--) {
        if(i < 0)
            nbytesHex[j] = '0';
        else
            nbytesHex[j] = nbytesHex[i];
        i--;
    }
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
    if(_port > 1024 && _port < 65535)
        return 1;
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
    char streamIdAux[BUFFER_SIZE];

    for (int i = 1; i < argc; ++i) {

        if(strcmp(argv[i], "-i") == 0) {
            i++;
            if(sscanf(argv[i], "%s", ipaddr) != 1) {
                printf("Error decoding ipaddr\n");
                exit(1);
            }
            //printf("ipaddr %s\n", ipaddr);
        }
        else if(strcmp(argv[i], "-t") == 0) {
            i++;
            if(sscanf(argv[i], "%s", tport) != 1) {
                printf("Error decoding tport\n");
                exit(1);
            }
            //printf("tport %s\n", tport);
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
            //printf("uport %s\n", uport);
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
            //printf("rsaddr %s, rsport %s\n", rsaddr, rsport);
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
            if((*tcpsessions) < 1) {
                printf("tcpsessions has to be greater than 1 \n");
                exit(1);
            }
            //printf("tcpsessions %d\n", *tcpsessions);
        }
        else if(strcmp(argv[i], "-n") == 0) {
            i++;
            if(sscanf(argv[i], "%d", bestpops) != 1) {
                printf("Error decoding bestpops\n");
                exit(1);
            }
            //printf("bestpops %d\n", *bestpops);
             if(*(bestpops) < 1){
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
            //printf("tsecs %d\n", *tsecs);
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
            printf("iamroot [<streamID>] [-i <ipaddr>] [-t <tport>] [-u <uport>] \n[-s <rsaddr>[:<rsport>]]\n[-p <tcpsessions>]\n[-n <bestpops>] [-x <tsecs>]\n[-b] [-d] [-h] \n");    
            exit(0);
        }
        else {
            if(sscanf(argv[i], "%s", streamIdAux) != 1) {
                printf("error in streamId \n");
                exit(1);
            }
            if(strlen(streamIdAux) > 63) {
                printf("the length of streamId is greater then 63 charact \n");
                return 1;
            }

            for(int j = 0; j< strlen(streamIdAux) ; j++) {
                streamId[j] = tolower(streamIdAux[j]);
            }
            streamId[strlen(streamIdAux)] = '\0';
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



int findsNewLine(char *buffer, int size) {
    for (int i = 0; i < size; ++i) {
        if(buffer[i] == '\n')
            return i;
    }
    return -1;
}

int findsDoubleNewLine(char *buffer, int size) {
    for (int i = 0; i < size; ++i) {
        if(buffer[i] == '\n')
            if(i + 1 < size)
                if(buffer[i+1] == '\n')
                    return i+1;
    }
    return -1;
}


