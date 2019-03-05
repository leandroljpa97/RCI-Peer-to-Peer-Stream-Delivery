#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "inout.h"


int checkPort(int _port){
    if(_port>1024 && _port <65535)
        return 1;
    return 0;
}


void readInputArguments(int argc, char* argv[], char streamId[], char streamName[],
    					char streamIP[], char streamPort[], char ipaddr[], 
						char  tport[], char uport[], char rsaddr[], char rsport[],
						int * tcpsessions, int * bestpops, int * tsecs, 
						int *dataStream, int *debug) {

    if(argc < 2) {
        printf("%d \n",argc);
        printf("without name of stream  \n");
        exit(1);
    }
    if(sscanf(argv[1], "%s", streamId) != 1) {
       printf("error in streamId \n");
       exit(1);
    }
    if(strlen(streamId) > 63) {
        printf("the length of streamId is greater then 63 charact \n");
        exit(1);
    }
    if(sscanf(streamId,"%[^:]:%[^:]:%s", streamName, streamIP, streamPort)!=3){
        printf("error in streamId parameters \n");
        exit(1);
    }
    // A porta que é ali indicada é a porta do servidor de stream, nao da nossa maquina logo aqui nao sabes se esta bem ou nao
    if(!checkPort(atoi(streamPort))){
        printf("Port not available \n");
        exit(1);
    }

    printf("o stream Name é : %s \n", streamName);
    printf("a streamIP é : %s \n", streamIP);
    printf("o porto é %s \n",streamPort);

    for (int i = 2; i < argc; ++i) {

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
                printf("Error decoding ip_root_server, rsport\n");
                exit(1);
            }
            printf("ip_root_server %s, rsport %s\n", rsaddr, rsport);
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
            printf("Wrong input argument: %s\n", argv[i]);
            exit(1);
        }
    }

    printf("Input Arguments Read\n");


}



