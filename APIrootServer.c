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
#include <signal.h>


#include "APIrootServer.h"
#include "APIaccessServer.h"
#include "APIpairComunication.h"
#include "utils.h"
#include "udp.h"
#include "tcp.h"

void ctrl_c_callback_handler(int signum) {
    printf("sai por CTRL_C \n ");

    if(root)
      REMOVE();

    closeAllClients();
    clearClientStructure();
    //temos de dar close aos sockets todos nao?
    exit(0);
}

void initializations() {
    void (*ctrl_c)(int);
    ctrl_c = signal(SIGINT, ctrl_c_callback_handler);

    struct sigaction act;

    memset(&act,0,sizeof(act));
    act.sa_handler = SIG_IGN;


    if(ctrl_c == SIG_ERR || (sigaction(SIGPIPE, &act, NULL) == -1))
        error_confirmation("Could not handle SIGINT or SIGPIPE");
}

void DadLeft(int * _root, int * _fdAccessServer, int * _fdUp){

    printf("Stream Stop .. Wait a moment! \n");

    status = DAD_LOST;
    close(*_fdUp);
    *_fdUp = -1;

    for(int j = 0; j < tcpsessions; j++)
        if(clients.fd[j] != 0)
            if(!BROKEN_STREAM(clients.fd[j]))
                removeChild(j);


    WHOISROOT(_root,_fdAccessServer,_fdUp);
    if(*_fdUp == -1){
        printf("My dad did not do REMOVE() \n");
        REMOVE();
        WHOISROOT(_root,_fdAccessServer,_fdUp);
        status = NORMAL;
    }
    printf("Stream recovered. Enjoy it \n");

    for(int j = 0; j < tcpsessions; j++)
        if(clients.fd[j] != 0)
            if(!STREAM_FLOWING(clients.fd[j]))
                removeChild(j);
}

int findDad(char _accessServerIP[], char _accessServerPort[], char _availableIAmRootIP[] , char _availableIAmRootPort[]){
    struct sockaddr_in addr;
    int fd = -1;
    fd_set fd_sockets;  
    struct timeval *t1 = NULL;
    struct timeval t2;
    char buffer[BUFFER_SIZE], buffer_aux[BUFFER_SIZE], action[BUFFER_SIZE];

    printf("o ip da root é %s \n", _accessServerIP);
    printf("o porto da root é %s \n", _accessServerPort);

    // Initiates UPD socket for communication with the accessServer
    struct addrinfo *res = createUPDsocket(&fd, _accessServerIP, _accessServerPort);

    // Tries 3 times to send POPREQ message to the AccessServer
    int tries = 0;
    int counter = 0;
    int max = fd;
    printf("PERFORM POPREQ\n");
    do {
        POPREQ(fd, res);
        printf("SENT POPREQ\n");

        // Indicate to select to watch UDP socket
        FD_ZERO(&fd_sockets);
        max = fd;
        addFd(&fd_sockets, &max, fd);

        // Time variables
        t1 = NULL;
        t2.tv_usec = 0;
        t2.tv_sec = TIMEOUT;
        t1 = &t2;
    
        // Puts server in receive state with timeout option
        counter = select(max + 1, &fd_sockets, (fd_set*) NULL, (fd_set *)NULL, (struct timeval*) t1);     
            
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
        printf("TRY POPREQ\n");
    } while(counter < 1 && tries < TRIES);

    if(tries >= TRIES) {
        // FUNCTION TO TURN OFF EVERYTHING
        perror("Error POPREQ"); 
        freeaddrinfo(res);
        close(fd);
        return 0;
    }

    printf("POPREQ msg sent \n");

    if(FD_ISSET(fd, &fd_sockets)){
        int n = receiveUdp(fd, buffer, BUFFER_SIZE, &addr);
        if(strstr(buffer, "POPRESP") != NULL) {
            n = sscanf(buffer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", 
                action, buffer_aux, _availableIAmRootIP , _availableIAmRootPort);
            if(n != 4) {
                printf("ERROR on POPRESP\n");
            }
        } 
        else {
            printf("ERROR on receiving to who to connect to\n");
            // EXIT PROGRAM
            exit(0);
        }

        printf("availableIAmRootIP : %s \n", _availableIAmRootIP);
                printf("availableIAmRootPort : %s  \n", _availableIAmRootPort);


        printf("recebi um popresp: %s\n", buffer);
    }

    freeaddrinfo(res);

    close(fd);

    return 1;
}



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

    // Time variables
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

    // finds the size of the WHOISROOT message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    // Tries 3 times to get the information from root server
    int tries = 0;
    int counter = 0;
    int max = fd;
    do {
        // Sends the information to the root server
        sendUdp(fd, buffer, i + 1, res);

        // Indicate to select to watch UDP socket
        FD_ZERO(&fd_sockets);
        max = fd;
        addFd(&fd_sockets, &max, fd);
    
        // Puts server in receive state with timeout option
        counter = select(max + 1, &fd_sockets, (fd_set*) NULL, (fd_set *)NULL, (struct timeval*) t1);     
            
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

    if(tries >= TRIES) {
        // FUNCTION TO TURN OFF EVERYTHING
        perror("Error wgo is root"); 
        freeaddrinfo(res);
        close(fd);
        exit(0);
    }

    if(FD_ISSET(fd, &fd_sockets)){
        struct sockaddr_in addr;

    	// Receives the response from the root server
        receiveUdp(fd, bufferRootServer, BUFFER_SIZE, &addr);
        sscanf(bufferRootServer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", action, stream, accessServerIP, accessServerPort);

        // If the tree is empty, the program is the root stream. Change the state to root and goes to connect to a stream
        if(!strcmp(action, "URROOT")){
            // Indicates that the program is the root of a tree
            printf("I am ROOT \n");
            *root = 1;

            // Creates Access Server
            *fdAccessServer = initUDPserver();

            // Access to stream to start transmission
            *fdUp = connectToStream();

            
        }
        // Receives the information that there's already a root on the tree 
        // and needs to go to the access server to acquire the correct IP and port
        else if(!strcmp(action, "ROOTIS")){                    
            *root = 0;

            char availableIAmRootIP[IP_SIZE], availableIAmRootPort[BUFFER_SIZE];

            // Communicates with access server to understand to where to connect
            
            /*if(findDad(accessServerIP, accessServerPort, availableIAmRootIP, availableIAmRootPort) == 0) {
                WHOISROOT(root, fdAccessServer, fdUp);
            } */
                
            findDad(accessServerIP, accessServerPort, availableIAmRootIP, availableIAmRootPort);   
            *fdUp = connectToTcp(availableIAmRootIP, availableIAmRootPort);

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
   /* strcpy(buffer, "REMOVE ");
    strcat(buffer, streamId);
    strcat(buffer, "\n"); */

    sprintf(buffer, "REMOVE %s\n", streamId);

    // finds the size of the WHOISROOT message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    // Tries 3 times to get the information from root server
    int counter = 0;

    // Sends the information to the root server
    sendUdp(fd, buffer, i + 1, res);

    // Indicate to select to watch UDP socket
    FD_ZERO(&fd_sockets);
    int max = fd;
    addFd(&fd_sockets, &max, fd);        

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
    else{
        if(FD_ISSET(fd, &fd_sockets)){
            struct sockaddr_in addr;

            // Receives the response from the root server
            receiveUdp(fd, bufferRootServer, BUFFER_SIZE, &addr);
            sscanf(bufferRootServer, "%s\n", action);
            printf("a action é: %s \n", action);
       
            if(!strcmp(action, "ERROR")) {
                printf("a mensagem de erro é %s\n", bufferRootServer);
            }
            else {
                printf("Error on the content of whoIsRoot, received %s\n", bufferRootServer);
                freeaddrinfo(res);
                close(fd);
                exit(0);
            }
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
	char *streams;

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

	// Sends the DUMP signal to the root server
    sendUdp(fd, "DUMP\n", strlen("DUMP\n"), res);

    // Tries 3 times to get the information from root server
    int tries = 0;
    int counter = 0;
    do {
        // Indicate to select to watch UDP socket
	    FD_ZERO(&fd_sockets);
	    int max = fd;
	    addFd(&fd_sockets, &max, fd);
	    
    
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

    if(tries >= TRIES) {
        // FUNCTION TO TURN OFF EVERYTHING
        perror("Error DUMP"); 
        freeaddrinfo(res);
        close(fd);
        return 0;
    }

    if(FD_ISSET(fd, &fd_sockets)) {
        struct sockaddr_in addr;

    	receiveUdp(fd, buffer, BUFFER_SIZE, &addr);

        // Advance the "STREAMS"
        streams = &buffer[8];
        printf("%s", streams);    	
	}

    freeaddrinfo(res);

    close(fd);

    return 1;
}


