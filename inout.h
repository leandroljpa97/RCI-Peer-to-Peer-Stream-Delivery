#ifndef INOUT_H_INCLUDE
#define INOUT_H_INCLUDE

int readInputArguments(int argc, char* argv[], char streamId[], char streamName[],
    					char streamIP[], char streamPort[], char ipaddr[], 
						char  tport[], char uport[], char rsaddr[], char rsport[],
						int * tcpsessions, int * bestpops, int * tsecs, 
						int *dataStream, int *debug);


#endif