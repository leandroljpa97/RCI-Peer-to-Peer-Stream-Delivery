#ifndef INOUT_H_INCLUDE
#define INOUT_H_INCLUDE

int checkPort(int _port);
/*
 * initMaskStdinFd: sets an empty mask of file descriptors to be 
 * controlled by select and activates sdantart input
 */
void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd);

/*
 * addFd: add a new file descriptor to be controlled by select
 */
void addFd(fd_set * _fd_sockets, int* _maxfd, int _fd);

int readInputArguments(int argc, char* argv[], char streamId[], char streamName[],
    					char streamIP[], char streamPort[], char ipaddr[], 
						char  tport[], char uport[], char rsaddr[], char rsport[],
						int * tcpsessions, int * bestpops, int * tsecs, 
						int *dataStream, int *debug);




#endif