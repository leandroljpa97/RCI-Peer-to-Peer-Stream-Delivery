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

void interpRootServerMsg(char _data[]) {
    char content[50];
    int flag = sscanf(_data, "%s", content);
    printf(" a mensagem vinda do stdi  é: %s \n",content);
    if (flag < 0){
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


int main(int argc, char const *argv[]) {

    initializations();
    // Indicates if the iamroot app is a root of the stream
	int root = 0;

    // Indicatse the number of the Query in 16 bits
    uint16_t _queryID = 0;
	
	// Files descriptor
    // fdUp enables TCP communication with the source (if this node is root) or with upper iamroot
	int fdAccessServer = -1;
	int fdUp = -1;
    int fdDown = -1;

	// Biggest Value of file descriptor pointer
    int maxfd = 0;

	// Mask of active file descriptors
    fd_set fd_sockets;  

    // Time variables for select timeout
    struct timeval * t1 = NULL;
    struct timeval t2;

    // Return state of the select
    int counter= 0;

	// Read Input Arguments of the program and set the default variables
    int dumpSignal = readInputArguments(argc, argv, streamId, streamName, streamIp, streamPort, ipaddr, tport, 
                       				uport, rsaddr, rsport, &tcpsessions, &bestpops, &tsecs, 
                      				&dataStream, &debug);

    printf(" o streamId: %s \n",streamId);

    // Init clients struct
    initClientStructure();

    // Print all available streams
    if(dumpSignal == 1) {
    	printf("No streamId. We suggest streams \n");
    	DUMP();
        exit(1);
    }
    
    printf("fdUp = %d\n", fdUp);
    printf("fdAccessServer = %d\n", fdAccessServer);

    WHOISROOT(&root, &fdAccessServer, &fdUp);

    printf("fiz who is root\n");
    printf("fdUp = %d\n", fdUp);
    printf("fdAccessServer = %d\n", fdAccessServer);

    // Create TCP Server
    fdDown = createTcpServer();

    while(1) {
    	// Inits the mask of file descriptor
        initMaskStdinFd(&fd_sockets, &maxfd);

        // Adds the file descriptor of the TCP to comm with root
        if(fdUp != -1)
            addFd(&fd_sockets, &maxfd, fdUp);

        // Adds the file descriptor for the communication with the access server
        if(fdAccessServer != -1)
            addFd(&fd_sockets, &maxfd, fdAccessServer);

        // Adds the file descriptor of the TCP to comm with clients
        if(fdDown != -1)
            addFd(&fd_sockets, &maxfd, fdDown);

        // Time variables
        t1 = NULL;
        t2.tv_usec = 0;
        t2.tv_sec = TIMEOUT;
        t1 = &t2;

        // Monitor all the file descritors to check for new inputs
        counter = select(maxfd+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
      
        if(counter < 0){
            perror("Error in select");
            
            for (int i = 0; i < tcpsessions; ++i)
            	close(clients.fd[i]);
            
            close(fdUp);
            close(fdAccessServer);
            exit(0);
        }

        if(!counter){
            // Select got timeout, reset
        }
        else {
        	// Checks if something was written on the standart input
            if(FD_ISSET(0, &fd_sockets)){
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
            else if(fdAccessServer != -1 && FD_ISSET(fdAccessServer, &fd_sockets)) {
                printf("Received something on the access server\n");

                char bufferAccessServer[BUFFER_SIZE];
                char availableIAmRootIP[BUFFER_SIZE];
                char availableIAmRootPort[BUFFER_SIZE];

                struct sockaddr_in addr;

                receiveUdp(fdAccessServer, bufferAccessServer, BUFFER_SIZE, &addr);

                if(strstr(bufferAccessServer, "POPREQ") != NULL) {
                    // If root has available connection, allow connection to itself
                    printf("RECEIVED A POPREQ\n");
                    if(clients.available > 0) {
                        // Sends POPRESP with IP and Port to connect to itself
                        POPRESP(fdAccessServer, &addr, ipaddr, tport);
                    }
                    else {
                        // Implementar procura na arvore
                    }
                }
            }
            // When receives a message from up on the tree
            else if(fdUp != -1 && FD_ISSET(fdUp, &fd_sockets)) {
                printf("i received something from TCP \n");

                if(root){
                    char bufferUp[PACKAGETCP];

                    int n = readTcp(fdUp, bufferUp, PACKAGETCP);

                    printf("o buffer tcp é: %s \n", bufferUp);
                    // Analise client list and send message
                    for (int i = 0; i < tcpsessions; ++i)                 {
                        if(clients.fd[i] != 0) {
                            DATA(clients.fd[i], n, bufferUp);
                        }
                    }
                }
                else if(!root){
                    printf("i received from my dad \n");

                    char bufferUp[TCP_MESSAGE_TYPE];
                    
                    if(readTcp(fdUp, bufferUp, TCP_MESSAGE_TYPE) != TCP_MESSAGE_TYPE) {
                        printf("Error reading message from dad\n");
                    }
                    
                    if(strcmp(bufferUp, "DA ") == 0) {
                        char sizeBuffer[] = "0000";
                        char bufferData[PACKAGETCP];
                        if(readTcp(fdUp, sizeBuffer, TCP_MESSAGE_SIZE) != TCP_MESSAGE_SIZE) {
                            printf("Error reading message from dad\n");
                        }
                        else if(readTcp(fdUp, bufferData, atoi(sizeBuffer)) != atoi(sizeBuffer)) {
                            printf("Error reading message from dad\n");
                        }
                        else {
                            printf("Receved stream %s\n", bufferData);

                            // Retransmit data to its clients
                            for (int i = 0; i < tcpsessions; ++i)                 {
                                if(clients.fd[i] != 0) {
                                    DATA(clients.fd[i], atoi(sizeBuffer), bufferUp);
                                }
                            }
                        }
                    }
                    else if(strcmp(bufferUp, "WE ") == 0) {
                        char bufferUpStreamId[TCP_MESSAGE_STREAMID];

                        if(readTcp(fdUp, sizeBuffer, TCP_MESSAGE_STREAMID) != TCP_MESSAGE_STREAMID) {
                            printf("Error reading message from dad\n");
                        }
                        else {
                            printf("Receved streamId %s", bufferUpStreamId);

                            if(strcmp(bufferUpStreamId, streamId) == 0) {
                                NEW_POP(fdUp);
                            }
                        }
                    }
                }                
            }
            // When receives a new client, performs accept
            else if(fdDown != -1 && FD_ISSET(fdDown,&fd_sockets)){
                printf("received newClient \n");

                // Variables to accept new clients
                int newfd = -1;
                struct sockaddr addr_tcpServer;
                unsigned int addrlenTcpServer = sizeof(struct sockaddr);

                if((newfd = accept(fdDown, (struct sockaddr *) &addr_tcpServer, &addrlenTcpServer)) == -1) {
                    printf("Error while accepting new client\n");
                    exit(1);
                }
                if(clients.available > 0){
                    if(insertFdClient(newfd, &clients)) {
                        // Sends the welcome message to the new client
                        if(!WELCOME(newfd)) {
                            deleteFdClient(newfd, &clients);
                        }
                        clients.available--;
                    }
                    else{
                        writeTcp(fdUp,"RE",strlen("RE"));

                    }
                }
                else {
                    writeTcp(fdUp,"RE",strlen("RE"));
                }


            } 
 
        }

    }

	return 0;
}