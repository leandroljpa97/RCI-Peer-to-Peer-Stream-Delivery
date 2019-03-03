#ifndef INOUT_H_INCLUDE
#define INOUT_H_INCLUDE

void readInputArguments(int argc, char* argv[], char ipaddr[],char tport[],char uport[],char ip_root_server[], char port_root_server[],
 		  	   int * tcpsessions, int * bestpops, int * tsecs,char streamId[64],char streamName[45],char sourceIp[46], 
 		  	   char sourcePort[],int *dataStream, int *debug);


#endif