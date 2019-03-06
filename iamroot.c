/**********************************************************************
Redes de Computadores e Internet - 2º Semestre
Authors: Francisco Mendes, Nº 84055
		 Leandro Almeida, Nº 84112
Last Updated on XXXXX
File name: rootComm.c
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

#include "APIrootServer.h"
#include "inout.h"
#include "udp.h"
#include "tcp.h"



//nao tenho a certeza se o porto do servidor fonte e este. nao enontrei no enunciado
#define MAX_LENGTH 200
#define BUFFSIZE 128
#define PACKAGETCP 200

#define TIMEOUT 30

#define NOSTATE -1
#define ROOTSERVER 1
#define FIND_UP 2
#define FIND_DAD 3
#define NORMAL 4



/*
 * initMaskStdinFd: sets an empty mask of file descriptors to be 
 * controlled by select and activates sdantart input
 */
void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd) {
	FD_ZERO(_fd_sockets);
	FD_SET(0,_fd_sockets);
	_maxfd = 0;

}

/*
 * addFd: add a new file descriptor to be controlled by select
 */
void addFd(fd_set * _fd_sockets, int* _maxfd, int _fd) {
    FD_SET(_fd, _fd_sockets);
    *_maxfd = _fd;
}





void interpRootServerMsg(int _fdRootServer, struct addrinfo *_res, char _data[], char _streamID[]){
    char content[50];
    int flag = sscanf(_data, "%s", content);
            printf(" a mensagem vinda do stdi  é: %s \n",content);
            if (flag<0){
                printf("Error reading stdin. Exit(0) the program\n");
                if(_fdRootServer!=-1) close(_fdRootServer);
                exit(0);
            }
            else{
                if(!strcmp(content,"streams")){
                    dump(_fdRootServer, _res);

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
                    Remove(_fdRootServer, _res, _streamID);
                }
                else 
                    printf("wrong command, try again \n");
            }

}


        

int main(int argc, char* argv[]){
    // Defualt init variables
    char streamId[64];
    char streamName[44];
    char streamIP[16];
    char streamPort[] = "00000";
    char ipaddr[16];
    char tport[] = "58000";
    char uport[] = "58000";
    char rsaddr[] = "193.136.138.142";
    char rsport[] = "59000";
    int tcpsessions = 1;
    int bestpops = 1;
    int tsecs = 5;
    int dataStream = 1;
    int debug = 0;
    int dumpSignal = 0;
        
    // Files descriptor
    // fdUp enables TCP communication with the source (if this node is root) or with upper iamroot
    int fdRootServer = -1, fdUp = -1;

    // Sizes of the addresses in upd and tcp communication
    int addrlen_udp, addrlen_tcp;
    
    // Mask of active file descriptors
    fd_set fd_sockets;  

    // Time variables for select timeout
    struct timeval* t1 = NULL;
    struct timeval t2;

    // State of the iamroot application
    int state = NOSTATE;

    // Return state of the select
    int counter= 0;

    // Biggest Value of file descriptor pointer
    int maxfd = 0;

    // Selects if app is root of the stream (is root = 1)
    int root = 0;

    // Aux variable
    int n;

    // IP Address and port of where the stream is coming
    char ipaddrRootStream[BUFFSIZE], uportRootStream[BUFFSIZE];

	//SOCKET UDP and TCP ! 
    struct addrinfo hints_tcp, *res_tcp, hints_udp;
    struct sockaddr_in addr_tcp, addr_udp;   

    // Read Input Arguments of the program and set the default variables
    dumpSignal = readInputArguments(argc, argv, streamId, streamName, streamIP, streamPort, ipaddr, tport, 
                        uport, rsaddr, rsport, &tcpsessions, &bestpops, &tsecs, 
                        &dataStream, &debug);

    // Initiate UPD and TCP strucuture details
    initUdp(&hints_udp);
    initTcp(&hints_tcp);


    // Initiates UPD Sockets for datagram communication
    struct addrinfo  *res = createUdpSocket(&fdRootServer, rsaddr, rsport, &hints_udp);

    // Print all available streams
    if(dumpSignal == 1) {
        dump(fdRootServer, res);
        char dumpBuffer[MAX_LENGTH];
        receiveUdp(fdRootServer, dumpBuffer, MAX_LENGTH, &addr_udp);

        printf("%s\n", dumpBuffer);
        exit(1);
    }

    // Communicates with the root server to check to how to connect with
    whoIsRoot(fdRootServer, res, streamId, ipaddr, uport);
    printf("fiz o whoIsRoot \n");

    state = ROOTSERVER;
    
    //confirmar se nao há um default para meter
    //valores um pouco a toa...         
    char action[50], userInput[MAX_LENGTH],content[65], buffer[BUFFSIZE], buffer_tcp[PACKAGETCP];
    
	while(1){	
        // Clean the buffers
        memset(buffer,0,sizeof(buffer));
        memset(buffer_tcp,0,sizeof(buffer_tcp));
        memset(action,0,sizeof(action));
        memset(userInput,0,sizeof(userInput));
        memset(content,0,sizeof(content));

		// Inits the mask of file descriptor
        initMaskStdinFd(&fd_sockets, &maxfd);

        // Adds the file descriptor for the sdandart input - UDP
        if(fdRootServer != -1)
            addFd(&fd_sockets, &maxfd, fdRootServer);

        // Adds the file descriptor of the TCP to comm with root
        if(fdUp!=-1)
            addFd(&fd_sockets, &maxfd, fdUp);

        // Time variables
		t1=NULL;
		t2.tv_usec = 0;
		t2.tv_sec = TIMEOUT;
		t1=&t2;

        // Monitor all the file descritors to check for new inputs
        counter = select(maxfd+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);     
        
        if(counter < 0){
            perror("Error in select");
            
            // CLOSE ALL FD

            exit(0);
        }

        // Select got timeout, reset
        if(!counter){
            printf("timeout!!\n");
            //se o state for root server tnh de retransmitir
        }

        // Checks if something was written on the standart input
        if(FD_ISSET(0, &fd_sockets)){
            if (fgets(userInput, MAX_LENGTH, stdin) == NULL)
                printf("Nothing to read in stdin.\n");
            else 
                interpRootServerMsg(fdRootServer, res, userInput, streamId);  
        
        }
        
        // Check if it receive communication from the Root Server
        else if(FD_ISSET(fdRootServer, &fd_sockets)){
            printf("recebi algo da root server \n");

            receiveUdp(fdRootServer, buffer, BUFFSIZE, &addr_udp);

            n = sscanf(buffer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", action,content , ipaddrRootStream ,uportRootStream);
            printf("a action é: %s \n",action);

            // If the tree is empty, the program is the root stream. Change the state to root and goes to connect to a stream
            if((!strcmp(action,"URROOT")) && state == ROOTSERVER){
                root = 1;
                state = FIND_UP;
            }
            // Receives the information that there's already a root on the tree 
            // and needs to go to the access server to acquire the correct IP and port
            else if(!strcmp(action,"ROOTIS")){
                //aqui o content ´e a stream id
                printf("a streamID é: %s \n", content);
                printf("o ip da root é %s \n",ipaddrRootStream);
                printf("o porto da root é %s \n",uportRootStream);
                
                if(state == ROOTSERVER){
                    state = FIND_UP;
                    root = 0;
                }

            }
            else if(!strcmp(action,"STREAMS")) {
                printf("as streams sao %s \n",content);
            }
            else if(!strcmp(action,"ERROR")) {
                //aqui a content é a mensagem de erro!!
                printf("a mensagem de erro é %s \n",content);
            }

            printf("buffer: %s\n",buffer); 

        }
        // When receives a message from up on the tree
        else if(FD_ISSET(fdUp, &fd_sockets)){
            printf("i received something from TCP \n");

            readTcp(fdUp, buffer_tcp, PACKAGETCP);

            printf("o buffer tcp é: %s \n", buffer_tcp);
            if(root){
                printf("i received stream and from sourceServer\n");
            }
            else if(!root){
                printf("i received from my dad \n");
            }
        } 

        // Depending on the result of "whoisroot", establishes a communication with the stream or the access server 
        if(state == FIND_UP){
            //establish communication with sourceServer, with ip and port obtained in streamId
            if(root){
                printf("VOU MORRER AQUI --------------------------------------------------------------------------------------- \n ");
                
                //o que se vai deixar é o de cima, mas meti o de baixo com o ncat, por isso a testares mete com o teu server!!
                n = getaddrinfo(streamIP, streamPort, &hints_tcp, &res_tcp);
                //n = getaddrinfo("192.168.2.10","58100",&hints_tcp, &res_tcp);
                if(n != 0) {
                    printf("error getaddrinfo in TCP source server \n");
                    exit(1);
                }

                fdUp = socket(res_tcp->ai_family, res_tcp->ai_socktype, res_tcp->ai_protocol);
                if(fdUp == -1) {
                    printf("error creating TCP socket TCP to source server!! \n ");
                    exit(1);
                }

                n = connect(fdUp, res_tcp->ai_addr, res_tcp->ai_addrlen);
                if(n == -1) {
                    printf("error in connect with TCP socket TCP in source!! \n ");
                    exit(1);
                }

                state = NORMAL; 
                printf("o fd depois do tcp é : %d \n", fdUp);

            }
            // if the node isn't root, it establish a connection with the root 
            else if(!root){
                //igual ao de cima!!! ->
                n = getaddrinfo(ipaddrRootStream , uportRootStream, &hints_tcp, &res_tcp);
                //n = getaddrinfo("192.168.1.7","58100",&hints_tcp, &res_tcp);

                if(n!=0) {
                    printf("error getaddrinfo in TCP source server \n");
                    exit(1);
                }

                fdUp = socket(res_tcp->ai_family, res_tcp->ai_socktype, res_tcp->ai_protocol);
                if(fdUp==-1) {
                    printf("error creating TCP socket TCP to source server!! \n ");
                    exit(1);
                 }

                n = connect(fdUp, res_tcp->ai_addr, res_tcp->ai_addrlen);
                if(fdUp==-1) {
                    printf("error in connect with TCP socket TCP in source!! \n "); 
                    exit(1);
                } 
                state=FIND_DAD;
            }
        }		
	}
	return 0;
}