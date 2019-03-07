#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "APIrootServer.h"
#include "udp.h"
#include "inout.h"
#define TIMEOUT 5


#define BUFFSIZE 200

void whoIsRoot(char _rsaddr[], char _rsport[], char _streamId[], char _ipaddr[], char _uport[], char _ipaddrRootStream[] , char _uportRootStream[]) {
    struct addrinfo hints;
    char buffer[BUFFSIZE];
    char bufferRootServer[BUFFSIZE];
    char buffer_aux[BUFFSIZE];
    char action[BUFFSIZE];
    int fd = -1 , max = -1, counter;
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

    //send WHOISROOT MSG
    strcpy(buffer,"WHOISROOT ");
    strcat(buffer,_streamId);
    strcat(buffer," ");
    strcat(buffer,_ipaddr);
    strcat(buffer,":");
    strcat(buffer,_uport);
    strcat(buffer,"\n");
    printf("o buffer no whoIsRoot é %s \n",buffer);
    
    sendUdp(fd, buffer, BUFFSIZE, res);

    //initialize File descriptors
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
        receiveUdp(fd, bufferRootServer, BUFFSIZE, &addr_udp);
        int n = sscanf(bufferRootServer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", action, buffer_aux, _ipaddrRootStream ,_uportRootStream);
        printf("a action é: %s \n",action);
         // If the tree is empty, the program is the root stream. Change the state to root and goes to connect to a stream
       
       /* if((!strcmp(action,"URROOT")) && state == ROOTSERVER){
            root = 1;
            
            state = FIND_UP;

            // Creates Access Server
            initUdpServer(&hints_accessServer);
            fdAccessServer = createUpdAccessServer(uport, &hints_accessServer); 
        } */
                // Receives the information that there's already a root on the tree 
                // and needs to go to the access server to acquire the correct IP and port
         if(!strcmp(action,"ROOTIS")){
                    //aqui o bufferRootServer ´e a stream id
            printf("a streamID é: %s \n", bufferRootServer);
            printf("o ip da root é %s \n",_ipaddrRootStream);
            printf("o porto da root é %s \n",_uportRootStream);
                    
            /*if(state == ROOTSERVER){
                state = FIND_UP;
                root = 0;
                    } */

                }
                
        else if(!strcmp(action,"ERROR")) {
            //aqui a bufferRootServer é a mensagem de erro!!
            printf("a mensagem de erro é %s \n",bufferRootServer);
                }

                printf("bufferRootServer: %s\n",bufferRootServer); 
     }
     close(fd);
}

void Remove( char _streamId[], char _rsaddr[], char _rsport[]){
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
    strcpy(buffer,"REMOVE ");
    strcat(buffer,_streamId);
    strcat(buffer,"\n");
    sendUdp(fd, buffer, BUFFSIZE, res);

    FD_ZERO(&fd_sockets);
    max = fd;
    addFd(&fd_sockets, &max, fd);

    //in remove we only check if there are response in case of error!
    counter = select(max+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
            
    if(counter < 0){
        perror("Error in select"); 
        close(fd);
        exit(0);
        }
    if(!counter){
        printf("Removed \n");
        return;
         }

    if(FD_ISSET(fd, &fd_sockets)){

            //testei isto metendo o buffer com tamanho 10 e buffersize 10 e deu mal... LOOL!
            n = receiveUdp(fd, buffer,BUFFSIZE, &addr_udp);
            printf("%s \n", buffer);
            return;
               
        }
    printf("removed! \n");

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

            //testei isto metendo o buffer com tamanho 10 e buffersize 10 e deu mal... LOOL!
            n = receiveUdp(fd, buffer,BUFFSIZE, &addr_udp);
            printf("%s", buffer);
            if(buffer[n-1] == '\n' && buffer[n-2] == '\n')
                break;
            buffer[0] = '\0';   
        }
    }

        close(fd);
    
   
}
