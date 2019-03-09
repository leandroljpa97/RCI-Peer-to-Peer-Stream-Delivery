/**********************************************************************
Redes de Computadores e Internet - 2º Semestre
Authors: Francisco Mendes, Nº 84055
		 Leandro Almeida, Nº 84112
Last Updated on XXXXX
File name: iamroot.c
COMMENTS
**********************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "utils.h"
#include "APIrootServer.h"
#include "APIpairComunication.h"
#include "APIaccessServer.h"
#include "udp.h"
#include "tcp.h"

#define PACKAGE_UDP 128
#define PACKAGE_TCP 128


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



void interpRootServerMsg(char _data[]) {
    char content[50];
    int flag = sscanf(_data, "%s", content);
            printf(" a mensagem vinda do stdi  é: %s \n",content);
            if (flag<0){
                printf("Error reading stdin. Exit(0) the program\n");
                
            }
            else{
                if(!strcmp(content,"streams")) {
                    DUMP();
                }
                else if(!strcmp(content,"state")){
                    printf("presses state \n");

                }
                else if(!strcmp(content,"display on")){
                    printf("pressed display on \n");

                }
                else if(!strcmp(content,"display off")){
                    printf("pressed display off \n");

                }
                else if(!strcmp(content,"format ascii")){
                    printf("pressed ascii \n");
                }
                else if(!strcmp(content,"format hex")){
                    printf("pressed display on \n");

                }
                else if(!strcmp(content,"debug on")){
                    printf("pressed debug on \n");

                }
                else if(!strcmp(content,"debug off")){
                    printf("pressed debug off \n");

                }
                else if(!strcmp(content,"tree")){
                    printf("pressed tree \n");

                }
                else if(!strcmp(content,"exit")){
                    printf("pressed exit \n");
                    REMOVE();
                }
                else 
                    printf("wrong command, try again \n");
            }

}

int main(int argc, char const *argv[])
{
	int root = 0;

    uint16_t _queryID = 0;
	
	// Files descriptor
    // fdUp enables TCP communication with the source (if this node is root) or with upper iamroot
	clients_t clients;
	int fdAccessServer = -1;
	int fdUp = -1;

	// Biggest Value of file descriptor pointer
    int maxfd = 0;

	// Mask of active file descriptors
    fd_set fd_sockets;  

    // Time variables for select timeout
    struct timeval* t1 = NULL;
    struct timeval t2;

    // Return state of the select
    int counter= 0;

	// Read Input Arguments of the program and set the default variables
    int dumpSignal = readInputArguments(argc, argv, streamId, streamName, streamIp, streamPort, ipaddr, tport, 
                       				uport, rsaddr, rsport, &tcpsessions, &bestpops, &tsecs, 
                      				&dataStream, &debug);

    clients.bestpops = tcpsessions;
    clients.available = 0;
    clients.fd = (int *) calloc(tcpsessions, sizeof(int));
    clients.mask = (int *) calloc(tcpsessions, sizeof(int));

    // Print all available streams
    if(dumpSignal == 1) {
    	printf("No streamId. We suggest streams \n");
    	DUMP();
        exit(1);
    }

    WHOISROOT(&root, &fdAccessServer, &fdUp);

    printf("fiz who is root\n");

    while(1) {
    	// Inits the mask of file descriptor
        initMaskStdinFd(&fd_sockets, &maxfd);

        // Adds the file descriptor for the communication with the access server
        if(fdAccessServer != -1)
            addFd(&fd_sockets, &maxfd, fdAccessServer);

        // Adds the file descriptor of the TCP to comm with root
        if(fdUp != -1)
            addFd(&fd_sockets, &maxfd, fdUp);

        // Time variables
		t1 = NULL;
		t2.tv_usec = 0;
		t2.tv_sec = TIMEOUT;
		t1 = &t2;

        // Monitor all the file descritors to check for new inputs
        counter = select(maxfd+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
        
        if(counter < 0){
            perror("Error in select");
            
            for (int i = 0; i < bestpops; ++i)
            	close(clients.fd[i]);
            
            close(fdUp);
            close(fdAccessServer);
            exit(0);
        }

        // Select got timeout, reset
        if(!counter){
            printf("timeout!!\n");
        }
        else {
        	// Checks if something was written on the standart input
            if(FD_ISSET(0, &fd_sockets)){
                printf("receive something stdin\n");
                char userInput[BUFFER_SIZE];
                // Clean the buffers
		    	memset(userInput,0,sizeof(userInput));
		        userInput[0] = '\0';

		        // Translates the message to the buffer
                if (fgets(userInput, BUFFER_SIZE, stdin) == NULL)
                    printf("Nothing to read in stdin.\n");
                else 
                    interpRootServerMsg(userInput);    
            } 
            // When receives messages from the access server
            else if(FD_ISSET(fdAccessServer, &fd_sockets)) {
                printf("Received something on the access server\n");

                char buffer[BUFFER_SIZE];
                char bufferAccessServer[BUFFER_SIZE];
                char actionAccessServer[BUFFER_SIZE];
                char availableIAmRootIP[BUFFER_SIZE];
                char availableIAmRootPort[BUFFER_SIZE];

                receiveUdp(fdAccessServer, bufferAccessServer, BUFFER_SIZE);

                if(strstr(bufferAccessServer, "POPREQ") != NULL) {

                        int status = POPREQ(clients);
                }
                else if(strstr(bufferAccessServer, "POPRESP") != NULL) {
                    if(sscanf(bufferAccessServer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", 
                       		  actionAccessServer, buffer, availableIAmRootIP , availableIAmRootPort) == 0)
                        printf("Error on message from access server\n");
                } 
            }
            // When receives a message from up on the tree
            else if(FD_ISSET(fdUp, &fd_sockets)){
                printf("i received something from TCP \n");

                char bufferUp[PACKAGETCP];

                readTcp(fdUp, bufferUp, PACKAGETCP);

                printf("o buffer tcp é: %s \n", bufferUp);
                if(root){
                    printf("i received stream and from sourceServer\n");
                }
                else if(!root){
                    printf("i received from my dad \n");
                }

                // Analise client list and send message
                for (int i = 0; i < bestpops; ++i)                 {
                	if(clients.mask[i] != 0) {
                		// retransmit message to client.fd[i]
                	}
                }
            } 
        }

    }

	return 0;
}