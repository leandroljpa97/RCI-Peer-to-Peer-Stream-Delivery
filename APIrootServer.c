#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "APIrootServer.h"
#include "utils.h"
#include "udp.h"
#include "tcp.h"


/*
 WHOISROOT: 
 Esta mensagem é enviada ao servidor de raízes, por uma aplicação iamroot, pedindo o
endereço IP e o porto UDP, do servidor de acesso da raiz da árvore de escoamento, associados
a um stream, identificado no pedido. Para além de, no pedido, ser identificado o stream, é
também indicado o endereço IP e o porto UDP do servidor de acesso da aplicação iamroot que
faz o pedido. Caso não haja nenhum registo no servidor de raízes associado ao stream, a
aplicação iamroot que faz o pedido ficará registada como raiz do stream em questão.
 

 */
int WHOISROOT(int *root, int *fdAccessServer, int *fdUp) {
	int fd;
	char buffer[BUFFER_SIZE];
	char stream[BUFFER_SIZE];
    char bufferRootServer[BUFFER_SIZE];
    char action[BUFFER_SIZE];
    char accessServerIP[BUFFER_SIZE];
    char accessServerPort[BUFFER_SIZE];

    // Mask for the select
    fd_set fd_sockets;  

    // Variables for select time out
    struct timeval* t1 = NULL, t2;

    t1 = NULL;
	t2.tv_usec = 0;
	t2.tv_sec = TIMEOUT;
	t1 = &t2;

	// Creates an UDP socket for communication with root server
	struct addrinfo * res = createUPDsocket(&fd, rsaddr, rsport);

	// Creates WHOISROOT message
    strcpy(buffer, "WHOISROOT ");
    strcat(buffer, streamId);
    strcat(buffer, " ");
    strcat(buffer, ipaddr);
    strcat(buffer, ":");
    strcat(buffer, uport);
    strcat(buffer, "\n");
    printf("o buffer no whoIsRoot é %s\n", buffer);

    // finds the size of the WHOISROOT message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    // Sends the information to the root server
    sendUdp(fd, buffer, i + 1, res);

    // Indicate to select to watch UDP socket
    FD_ZERO(&fd_sockets);
    int max = fd;
    addFd(&fd_sockets, &max, fd);
    
    // Tries 3 times to get the information from root server
    int tries = 0;
    int counter = 0;
    do {
        // Puts server in receive state with timeout option
        counter = select(max+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
            
        if(counter < 0){
            perror("Error in select"); 
            close(fd);
            exit(0);
            }
        // if counter = 0, any response was received
        if(!counter){
            printf("timeout...\n");
        }
        tries++;
    } while(counter < 1 && tries < TRIES);

    if(FD_ISSET(fd, &fd_sockets)){
    	// Receives the response from the root server
        receiveUdp(fd, bufferRootServer, BUFFER_SIZE);
        sscanf(bufferRootServer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", action, stream, accessServerIP ,accessServerPort);
        printf("a action é: %s \n", action);

        // If the tree is empty, the program is the root stream. Change the state to root and goes to connect to a stream
        if(!strcmp(action, "URROOT")){
            // Indicates that the program is the root of a tree
            *root = 1;

            // Creates Access Server
            *fdAccessServer = initUDPserver();

            // Access to stream to start transmission
            *fdUp = connectToTcp();
        }
        // Receives the information that there's already a root on the tree 
        // and needs to go to the access server to acquire the correct IP and port
        else if(!strcmp(action, "ROOTIS")){
            printf("a streamID é: %s \n", stream);
            printf("o ip da root é %s \n", accessServerIP);
            printf("o porto da root é %s \n", accessServerPort);
                    
            *root = 0;

            // Connect to access server - implement API
        }
        else if(!strcmp(action, "ERROR")) {
            //aqui a bufferRootServer é a mensagem de erro!!
            printf("a mensagem de erro é %s \n",bufferRootServer);
        }
        else {
            printf("Error on the content of whoIsRoot\n");
            exit(0);
        }

    }

    freeaddrinfo(res);

    close(fd);

    return 1;
}

int REMOVE() {
	// File descriptor to communicate with root server
	int fd = -1;

	// Buffers to hold messages
	char buffer[BUFFER_SIZE];
    char bufferRootServer[BUFFER_SIZE];
    char action[BUFFER_SIZE];

    // Mask for the select
    fd_set fd_sockets;  

    // Variables for select time out
    struct timeval* t1 = NULL, t2;

    t1 = NULL;
	t2.tv_usec = 0;
	t2.tv_sec = TIMEOUT_REMOVE;
	t1 = &t2;

	// Creates an UDP socket for communication with root server
	struct addrinfo * res = createUPDsocket(&fd, rsaddr, rsport);

	// Creates remove message with the idication of current StreamID
    strcpy(buffer, "REMOVE ");
    strcat(buffer, streamId);
    strcat(buffer, "\n");

    // finds the size of the WHOISROOT message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    // Sends the information to the root server
    sendUdp(fd, buffer, i + 1, res);

    // Indicate to select to watch UDP socket
    FD_ZERO(&fd_sockets);
    int max = fd;
    addFd(&fd_sockets, &max, fd);
    
    // Tries 3 times to get the information from root server
    int tries = 0;
    int counter = 0;
    do {
        // Puts server in receive state with timeout option
        counter = select(max+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
            
        if(counter < 0){
            perror("Error in select"); 
            close(fd);
            exit(0);
            }
        // if counter = 0, any response was received
        if(!counter){
            printf("timeout... No erors on remove\n");
        }
        tries++;
    } while(counter < 1 && tries < TRIES);

    if(FD_ISSET(fd, &fd_sockets)){
    	// Receives the response from the root server
        receiveUdp(fd, bufferRootServer, BUFFER_SIZE);
        sscanf(bufferRootServer, "%s\n", action);
        printf("a action é: %s \n", action);
   
        if(!strcmp(action, "ERROR")) {
            printf("a mensagem de erro é %s\n", bufferRootServer);
        }
        else {
            printf("Error on the content of whoIsRoot, received %s\n", bufferRootServer);
            exit(0);
        }
    }

    freeaddrinfo(res);

    close(fd);

    return 1;
}


int DUMP() {
	// File descriptor to communicate with root server
	int fd = -1;

	// Buffers to hold messages
	char buffer[BUFFER_SIZE];
	char streams[BUFFER_SIZE];

    // Mask for the select
    fd_set fd_sockets;  

    // Variables for select time out
    struct timeval* t1 = NULL, t2;

    t1 = NULL;
	t2.tv_usec = 0;
	t2.tv_sec = TIMEOUT_REMOVE;
	t1 = &t2;

	// Creates an UDP socket for communication with root server
	struct addrinfo * res = createUPDsocket(&fd, rsaddr, rsport);

	// Sends the DUMP signal to the root server
    sendUdp(fd, "DUMP\n", strlen("DUMP\n"), res);

    int count = 0;

    while(1) {
	    // Indicate to select to watch UDP socket
	    FD_ZERO(&fd_sockets);
	    int max = fd;
	    addFd(&fd_sockets, &max, fd);
	    
	    // Tries 3 times to get the information from root server
	    int tries = 0;
	    int counter = 0;
	    do {
	        // Puts server in receive state with timeout option
	        counter = select(max+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
	            
	        if(counter < 0){
	            perror("Error in select"); 
	            close(fd);
	            exit(0);
	            }
	        // if counter = 0, any response was received
	        if(!counter){
	            printf("timeout dump...\n");
	        }
	        tries++;
	    } while(counter < 1 && tries < TRIES);

	    if(FD_ISSET(fd, &fd_sockets)){
	    	int n = receiveUdp(fd, buffer, BUFFER_SIZE);

	    	if(count == 0) {
				sscanf(buffer, "%[^\n ] %[^ ]", streams, buffer);
        
	    	}
	        printf("%s", buffer);
	        if(buffer[n-1] == '\n' && buffer[n-2] == '\n')
	            break;
	        buffer[0] = '\0';   
	        count++;
	    }

	}
	printf("\n");
    freeaddrinfo(res);

    close(fd);

    return 1;
}


