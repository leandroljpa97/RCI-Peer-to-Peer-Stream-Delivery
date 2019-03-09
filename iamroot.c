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
	int root = 0;

    uint16_t _queryID = 0;
	
	// Files descriptor
    // fdUp enables TCP communication with the source (if this node is root) or with upper iamroot
	clients_t clients;
	int fdAccessServer = -1;
	int fdUp = -1;
    int fdDown = -1;

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

    // Init clients struct
    clients.available = tcpsessions;
    clients.fd = (int *) calloc(tcpsessions, sizeof(int));

    // Print all available streams
    if(dumpSignal == 1) {
    	printf("No streamId. We suggest streams \n");
    	DUMP();
        exit(1);
    }

    WHOISROOT(&root, &fdAccessServer, &fdUp);

    printf("fiz who is root\n");

    // Create TCP Server
    fdDown = createTcpServer();

    while(1) {
    	// Inits the mask of file descriptor
        initMaskStdinFd(&fd_sockets, &maxfd);

        // Adds the file descriptor for the communication with the access server
        if(fdAccessServer != -1)
            addFd(&fd_sockets, &maxfd, fdAccessServer);

        // Adds the file descriptor of the TCP to comm with root
        if(fdUp != -1)
            addFd(&fd_sockets, &maxfd, fdUp);

        // Adds the file descriptor of the TCP to comm with clients
        if(fdDown != -1)
            addFd(&fd_sockets, &maxfd, fdDown);

        // Time variables
        setTimeOut(t1, &t2);

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
            else if(fdAccessServer != -1 && FD_ISSET(fdAccessServer, &fd_sockets)) {
                printf("Received something on the access server\n");

                char buffer[BUFFER_SIZE];
                char bufferAccessServer[BUFFER_SIZE];
                char actionAccessServer[BUFFER_SIZE];
                char availableIAmRootIP[BUFFER_SIZE];
                char availableIAmRootPort[BUFFER_SIZE];

                struct sockaddr_in addr;

                receiveUdp(fdAccessServer, bufferAccessServer, BUFFER_SIZE, &addr);

                if(strstr(bufferAccessServer, "POPREQ") != NULL) {
                    // If root has available connection, allow connection to itself
                    if(clients.available > 0) {
                        POPRESP(fdAccessServer, &addr, ipaddr, uport);
                    }
                    else {
                        // Implementar procura na arvore
                    }
                }
            }
            // When receives a message from up on the tree
            else if(fdUp != -1 && FD_ISSET(fdUp, &fd_sockets)) {
                printf("i received something from TCP \n");

                char bufferUp[PACKAGETCP];

                // Clean the buffers
                memset(bufferUp,0, sizeof(bufferUp));
                bufferUp[0] = '\0';

                readTcp(fdUp, bufferUp, PACKAGETCP);

                printf("o buffer tcp é: %s \n", bufferUp);
                if(root){
                    printf("i received stream and from sourceServer\n");
                    // Analise client list and send message
                    for (int i = 0; i < tcpsessions; ++i)                 {
                        if(clients.fd[i] != 0) {
                            // retransmit message to client.fd[i]
                           /* strcpy(bufferStream, "DATA ");
                            DecToHexStr(strlen(bufferUp)-1, strHexa);
                            strcat(bufferStream,strHexa);
                            strHexa[0] = '\0';
                            strcat(bufferStream,"\n");
                            strcat(bufferStream,bufferUp);
                            sendStreamToChilds(bufferStream,fdClients, tcpsessions);
                            printf("i received stream and from sourceServer\n");*/
                        }
                    }
                }
                else if(!root){
                    printf("i received from my dad \n");
                    /*n = sscanf(bufferUp,"%[^ ] %[^\n]\n %s",actionData,strHexa,contentStream);
                    if(n == 3 && !strcmp(actionData,"DATA")){
                            printf("%s \n",contentStream);
                            sendStreamToChilds(bufferStream,fdClients, tcpsessions);

                        }*/
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