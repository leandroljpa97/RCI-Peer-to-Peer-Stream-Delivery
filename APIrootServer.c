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
#include "udp.h"
#include "tcp.h"
#include "inout.h"

#define TIMEOUT 2
#define BUFFSIZE 200
#define TRIES 3

#define NOSTATE -1
#define ROOTSERVER 1
#define FIND_UP 2
#define FIND_DAD 3
#define NORMAL 4


/*
 whoIsRoot: 
 Esta mensagem é enviada ao servidor de raízes, por uma aplicação iamroot, pedindo o
endereço IP e o porto UDP, do servidor de acesso da raiz da árvore de escoamento, associados
a um stream, identificado no pedido. Para além de, no pedido, ser identificado o stream, é
também indicado o endereço IP e o porto UDP do servidor de acesso da aplicação iamroot que
faz o pedido. Caso não haja nenhum registo no servidor de raízes associado ao stream, a
aplicação iamroot que faz o pedido ficará registada como raiz do stream em questão.
 
 return: 
        -1 = error
        0 = ROOTIS
        1 = URROOT
 */
int whoIsRoot(char _rsaddr[], char _rsport[], char _streamId[], char _streamIp[], 
              char _streamPort[], char _ipaddr[], char _uport[], char _ipaddrRootStream[], 
              char _uportRootStream[], int *state, int *root, struct addrinfo *hints_accessServer, 
              struct addrinfo *hints_tcp, struct addrinfo **res_tcp, int *fdAccessServer, int *fdUp) {

    struct addrinfo hints;
    char buffer[BUFFSIZE];
    char bufferRootServer[BUFFSIZE];
    char buffer_aux[BUFFSIZE];
    char action[BUFFSIZE];
    int fd = -1 , max = -1, counter, tries = 0;
    struct sockaddr_in addr_udp;
    fd_set fd_sockets;  
    struct timeval* t1 = NULL;
    struct timeval t2;

    int returnState = -1;

    t1 = NULL;
    t2.tv_usec = 0;
    t2.tv_sec = TIMEOUT;
    t1 = &t2;

    // Initiates UPD socket for communication with the root server 
    initUdp(&hints);
    struct addrinfo  *res = createUdpSocket(&fd, _rsaddr, _rsport, &hints);

    // Creates WHOISROOT message
    strcpy(buffer,"WHOISROOT ");
    strcat(buffer,_streamId);
    strcat(buffer," ");
    strcat(buffer,_ipaddr);
    strcat(buffer,":");
    strcat(buffer,_uport);
    strcat(buffer,"\n");
    printf("o buffer no whoIsRoot é %s \n",buffer);
    
    // Sends the information to the root server
    sendUdp(fd, buffer, BUFFSIZE, res);

    // Indicate to select to watch UDP socket
    FD_ZERO(&fd_sockets);
    max = fd;
    addFd(&fd_sockets, &max, fd);

    // Tries 3 times to get the information from root server
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
        receiveUdp(fd, bufferRootServer, BUFFSIZE, &addr_udp);
        sscanf(bufferRootServer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", action, buffer_aux, _ipaddrRootStream ,_uportRootStream);
        printf("a action é: %s \n",action);
        
        // If the tree is empty, the program is the root stream. Change the state to root and goes to connect to a stream
        if((!strcmp(action,"URROOT")) && *state == ROOTSERVER){
            // Indicates that the program is the root of a tree
            *root = 1;
            
            *state = FIND_UP;

            // Creates Access Server
            initUdpServer(hints_accessServer);
            *fdAccessServer = createUpdAccessServer(_uport, hints_accessServer);

            // Access to stream to start transmission
            *fdUp = connectToTcp(_streamIp, _streamPort, hints_tcp, res_tcp);
            returnState = 1; 
        }
        // Receives the information that there's already a root on the tree 
        // and needs to go to the access server to acquire the correct IP and port
        else if(!strcmp(action,"ROOTIS")){
                    //aqui o bufferRootServer ´e a stream id
            printf("a streamID é: %s \n", bufferRootServer);
            printf("o ip da root é %s \n",_ipaddrRootStream);
            printf("o porto da root é %s \n",_uportRootStream);
                    
            if(*state == ROOTSERVER){
                *state = FIND_UP;
                *root = 0;
            }
            returnState = 0;
        }
                
        else if(!strcmp(action,"ERROR")) {
            //aqui a bufferRootServer é a mensagem de erro!!
            printf("a mensagem de erro é %s \n",bufferRootServer);
            returnState = -1;
        }

        else {
            printf("Error on the content of whoIsRoot\n");
            exit(0);
        }

        printf("bufferRootServer: %s\n",bufferRootServer); 
    }
    close(fd);

    return returnState;
}

void Remove( char _streamId[], char _rsaddr[], char _rsport[]){
    struct addrinfo hints;
    char buffer[BUFFSIZE];
    int fd = -1;

    // Initiates UPD socket for communication with the root server
    initUdp(&hints);
    struct addrinfo  *res = createUdpSocket(&fd, _rsaddr, _rsport, &hints);
    
    // Creates remove message with the idication of current StreamID
    strcpy(buffer,"REMOVE ");
    strcat(buffer,_streamId);
    strcat(buffer,"\n");

    // Send REMOVE message to the root server
    sendUdp(fd, buffer, BUFFSIZE, res);

    printf("remove message sent \n");

    close(fd);
 }


void dump(char _rsaddr[], char _rsport[]) {
    struct addrinfo hints;
    char buffer[BUFFSIZE];
    int fd = -1 , max = -1, counter, n;
    struct sockaddr_in addr_udp;
    fd_set fd_sockets;  
    struct timeval* t1 = NULL;
    struct timeval t2;

    t1 = NULL;
    t2.tv_usec = 0;
    t2.tv_sec = TIMEOUT;
    t1 = &t2;

    initUdp(&hints);
    struct addrinfo  *res = createUdpSocket(&fd, _rsaddr, _rsport, &hints);

    printf(" rsaddr: %s , _rsport: %s \n", _rsaddr, _rsport);

    
    sendUdp(fd, "DUMP\n", strlen("DUMP\n"), res);

    //there are strems very larges, so that we need to read until we get 2 \n 
    while(1){

        FD_ZERO(&fd_sockets);
        max = fd;
        addFd(&fd_sockets, &max, fd);
        counter = select(max+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
            
        if(counter < 0){
            perror("Error in select"); 
            close(fd);
            exit(0);
            }
        if(!counter){
            printf("timeout...\n");

        }

        if(FD_ISSET(fd, &fd_sockets)){
            n = receiveUdp(fd, buffer,BUFFSIZE, &addr_udp);
            printf("%s", buffer);
            if(buffer[n-1] == '\n' || buffer[n-2] == '\n')
                break;
            buffer[0] = '\0';   
        }
    }

    close(fd);
    
   
}
