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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <math.h>






#include "APIrootServer.h"
#include "inout.h"
#include "udp.h"
#include "tcp.h"





//nao tenho a certeza se o porto do servidor fonte e este. nao enontrei no enunciado
#define MAX_LENGTH 200
#define BUFFSIZE 128
#define PACKAGETCP 200

#define TIMEOUT 5

#define NOSTATE -1
#define ROOTSERVER 1
#define FIND_UP 2
#define FIND_DAD 3
#define ACCEPT_CHILD 4
#define NORMAL 5

void ctrl_c_callback_handler(int signum)
{
    printf("sai por CTRL_C \n ");
    exit(0);

}

void error_confirmation(char*s)
{
        printf("%s \n",s);
        exit(EXIT_FAILURE);
}

void initializations(){
    void (*ctrl_c)(int);
    ctrl_c = signal(SIGINT, ctrl_c_callback_handler);

    void (*close_socket)(int);
    close_socket = signal(SIGPIPE, SIG_IGN);

    if(ctrl_c == SIG_ERR || close_socket == SIG_ERR)
        error_confirmation("Could not handle SIGINT or SIGPIPE");


}

int insertFdClient(int _newfd, int * _fdClients, int _tcpsessions){

    for(int i = 0; i< _tcpsessions; i++){
        if(_fdClients[i] == 0){
            _fdClients[i] = _newfd;
            return 1;
        }
    }
    return 0;

}




void findDad(char _ipaddr[], char _uport[], char _availableIAmRootIP[] , char _availableIAmRootPort[]){


    struct addrinfo hints;
    struct sockaddr_in addr;
    int fd = -1, max, counter;
    fd_set fd_sockets;  
    struct timeval* t1 = NULL;
    struct timeval t2;
    char buffer[MAX_LENGTH], buffer_aux[MAX_LENGTH],action[MAX_LENGTH];

    t1 = NULL;
    t2.tv_usec = 0;
    t2.tv_sec = TIMEOUT;
    t1 = &t2;

    // Initiates UPD socket for communication with the accessServer
    initUdp(&hints);
    struct addrinfo  *res = createUdpSocket(&fd, _ipaddr, _uport, &hints);

    // Send POPREQ message to the AccessServer
    sendUdp(fd, "POPREQ\n", strlen("POPREQ\n"), res);

    printf("POPREQ msg sent \n");


    printf("dentro do findDad-> ipaddr: %s e uport: %s \n", _ipaddr,_uport);
    //there are strems very larges, so that we need to read until we get 2 \n 
  

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
            int n = receiveUdp(fd, buffer,MAX_LENGTH, &addr);
            if(strstr(buffer, "POPRESP") != NULL) {
                    n = sscanf(buffer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", 
                        action, buffer_aux, _availableIAmRootIP , _availableIAmRootPort);
                } 


                printf("recebi um popresp \n");
                printf("_availableIAmRootPort:%s availableIAmRootPort: %s \n",_availableIAmRootIP , _availableIAmRootPort );

            printf("%s", buffer);  
        }
    




    close(fd);
}







void interpRootServerMsg(char _data[], char _streamID[], char _rsaddr[], char _rsport[]){
    char content[50];
    int flag = sscanf(_data, "%s", content);
            printf(" a mensagem vinda do stdi  é: %s \n",content);
            if (flag<0){
                printf("Error reading stdin. Exit(0) the program\n");
                
            }
            else{
                if(!strcmp(content,"streams"))
                    dump(_rsaddr, _rsport);

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
                    Remove(_streamID,_rsaddr, _rsport);
                }
                else 
                    printf("wrong command, try again \n");
            }

}

void initMaskStdinFd(fd_set * _fd_sockets, int* _maxfd) {
    FD_ZERO(_fd_sockets);
    FD_SET(0,_fd_sockets);
    _maxfd = 0;

}

/*
 * addFd: add a new file descriptor to be controlled by select
 */
void addFd(fd_set * _fd_sockets, int * _maxfd, int _fd) {
    FD_SET(_fd, _fd_sockets);
    *_maxfd = _fd;
}

void DecToHexStr(int dec, char *str){
 sprintf(str, "%X", dec);
}

void sendStreamToChilds(char * _bufferStream, int * _fdClients, int _tcpsessions){
    for(int i = 0; i < _tcpsessions; i++){
        printf("_fdClients[%d] é %d \n",i,_fdClients[i]);
        if(_fdClients[i] != 0){
            writeTcp(_fdClients[i], _bufferStream, strlen(_bufferStream));
        }
    }

}


int main(int argc, char* argv[]){
    // Defualt init variables
    char streamId[64];
    char streamName[44];
    char streamIP[16] = "193.136.138.142";
    char streamPort[] = "58011";
    char ipaddr[16] = "194.210.156.123";
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
    
    int clientsAvailable = 1;        
    // Files descriptor
    // fdUp enables TCP communication with the source (if this node is root) or with upper iamroot
    int fdUp = -1, fdAccessServer = -1, fdTcpServer = -1;
    int newfd;
    unsigned int addrlenTcpServer = sizeof(struct sockaddr);
    
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

    char availableIAmRootIP[16];
    char availableIAmRootPort[6];

    char strHexa[20];
    int sizeStream;
    char contentStream[BUFFSIZE];

	//SOCKET UDP and TCP ! 
    struct addrinfo hints_tcp, *res_tcp, hints_accessServer, hints_tcpServer, *res_tcpServer;
    struct sockaddr_in addr_udp, addr_tcpServer;    

    initializations();

    // Read Input Arguments of the program and set the default variables
    dumpSignal = readInputArguments(argc, argv, streamId, streamName, streamIP, streamPort, ipaddr, tport, 
                        uport, rsaddr, rsport, &tcpsessions, &bestpops, &tsecs, 
                        &dataStream, &debug);

   clientsAvailable = tcpsessions;
   int * fdClients = (int*)malloc(tcpsessions*sizeof(int));
   if(fdClients == NULL){
     printf("error alocating vector fdClients \n");
     exit(0);
   }
    for (int i = 0; i < tcpsessions; i++)
        fdClients[i] = 0;


    // Initiate TCP strucuture details
    initTcp(&hints_tcp);
    initTcpServer(&hints_tcpServer);

    // Print all available streams
    if(dumpSignal == 1) {
        dump(rsaddr, rsport);
        printf("No streamId. We suggest streams \n");

        exit(1);
    }

    state = ROOTSERVER;

    // Communicates with the root server to check to how to connect with
    whoIsRoot(rsaddr, rsport, streamId, streamIP, streamPort, ipaddr, uport, ipaddrRootStream , uportRootStream, &state, &root, &hints_accessServer, &hints_tcp, res_tcp, &fdAccessServer, &fdUp);
    printf("fiz o whoIsRoot - root %d, state %d\n", root, state);

    if(state == FIND_UP){
                //establish communication with sourceServer, with ip and port obtained in streamId
                if(root){
                    printf("VOU MORRER AQUI --------------------------------------------------------------------------------------- \n ");
                    printf("stream %s:%s\n", streamIP, streamPort);
                    //o que se vai deixar é o de cima, mas meti o de baixo com o ncat, por isso a testares mete com o teu server!!
                    
                    
                    //int nb = getaddrinfo("194.210.156.123", streamPort, &hints_tcp, &res_tcp);
                    int nb = getaddrinfo(streamIP, streamPort, &hints_tcp, &res_tcp);
                    if(nb != 0) {
                        printf("error getaddrinfo in TCP source server \n");
                        exit(1);
                     }

                    fdUp = socket((res_tcp)->ai_family, (res_tcp)->ai_socktype, (res_tcp)->ai_protocol);
                    if(fdUp == -1) {
                        printf("error creating TCP socket TCP to source server...2. \n ");
                        exit(1);
                    }

                    nb = connect(fdUp, (res_tcp)->ai_addr, (res_tcp)->ai_addrlen);
                    if(n == -1) {
                        printf("error in connect with TCP socket TCP in source.... \n ");
                        exit(1);
                    }

                    state = ACCEPT_CHILD; 
                    printf("o fd depois do tcp é : %d \n", fdUp);

                }

                // if the node isn't root, it establish a connection with the root 
                else if(!root){
                    findDad(ipaddrRootStream, uportRootStream, availableIAmRootIP , availableIAmRootPort);                    

                    int nb = getaddrinfo(availableIAmRootIP, availableIAmRootPort, &hints_tcp, &res_tcp);
                    if(nb != 0) {
                        printf("error getaddrinfo in TCP source server \n");
                        exit(1);
                     }

                    fdUp = socket((res_tcp)->ai_family, (res_tcp)->ai_socktype, (res_tcp)->ai_protocol);
                    if(fdUp == -1) {
                        printf("error creating TCP socket TCP to source server...2. \n ");
                        exit(1);
                    }

                    nb = connect(fdUp, (res_tcp)->ai_addr, (res_tcp)->ai_addrlen);
                    if(n == -1) {
                        printf("error in connect with TCP socket TCP in source.... \n ");
                        exit(1);
                    }


                    state = ACCEPT_CHILD; 
                    printf("o fd depois do tcp é : %d \n", fdUp);

                }  
             }

             printf("fdUp: %d \n",fdUp);
 

    // Communication buffers        
    char buffer[MAX_LENGTH], bufferStream[MAX_LENGTH];
    char userInput[MAX_LENGTH];
    
    char bufferUp[PACKAGETCP];

    // Buffers for Access Server Comunication
    char bufferAccessServer[MAX_LENGTH];
    char actionAccessServer[50];
    //send to new pair
    char bufferWelcome[BUFFSIZE];
    char actionData[20];
   
    
	while(1){

        if(state == ACCEPT_CHILD){
            int nTcp = getaddrinfo(NULL, tport, &hints_tcpServer, &res_tcpServer);
            if(nTcp!=0) {
                printf("Error getting addr info\n");
                exit(1);
            }
            printf("o tport é %s \n",tport);

            fdTcpServer = socket(res_tcpServer->ai_family, res_tcp->ai_socktype, res_tcpServer->ai_protocol);
            if(fdTcpServer==-1) {
                printf("Error creating socket\n");
                exit(1);
            }

            nTcp = bind(fdTcpServer, res_tcpServer->ai_addr, res_tcpServer->ai_addrlen);
            if(nTcp == -1) {
                printf("Error in bind on TCP Server\n");
                exit(1);
            }

            if(listen(fdTcpServer, bestpops)==-1) {
                printf("Error listen\n");
                exit(1);
            }
            printf("waiting new clients \n");
            state = NORMAL;
	
        }

        // Clean the buffers
        memset(buffer,0,sizeof(buffer));
        buffer[0] = '\0';
        memset(bufferUp,0,sizeof(bufferUp));
        bufferUp[0] = '\0';
        //memset(action,0,sizeof(action));
        memset(userInput,0,sizeof(userInput));
        userInput[0] = '\0';
        memset(bufferStream,0,sizeof(userInput));
        bufferStream[0] = '\0';
        
		// Inits the mask of file descriptor
        initMaskStdinFd(&fd_sockets, &maxfd);

        // Adds the file descriptor for the communication with the access server
        if(fdAccessServer != -1)
            addFd(&fd_sockets, &maxfd, fdAccessServer);
        

        // Adds the file descriptor of the TCP to comm with root
        if(fdUp != -1)
            addFd(&fd_sockets, &maxfd, fdUp);
        

        if(fdTcpServer != -1)
            addFd(&fd_sockets, &maxfd, fdTcpServer);

        // Time variables
		t1 = NULL;
		t2.tv_usec = 0;
		t2.tv_sec = TIMEOUT;
		t1 = &t2;

        // Monitor all the file descritors to check for new inputs
        counter = select(maxfd+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*) t1);  
        printf("counter : %d \n",counter);   
        
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
        else{

            // Checks if something was written on the standart input
            if(FD_ISSET(0, &fd_sockets)){
                printf("receive something stdin \n");
                if (fgets(userInput, MAX_LENGTH, stdin) == NULL)
                    printf("Nothing to read in stdin.\n");
                else 
                    interpRootServerMsg(userInput, streamId, rsaddr, rsport);    
            }

            // When receives messages from the access server
                 else if( fdAccessServer!= -1 && FD_ISSET(fdAccessServer, &fd_sockets)) {
                printf("Received something on the access server\n");

                receiveUdp(fdAccessServer, bufferAccessServer, MAX_LENGTH, &addr_udp);
                n = sscanf(bufferAccessServer, "%[^\n]", actionAccessServer);
                if(!strcmp(actionAccessServer,"POPREQ")){
                    //apenas para teste
                    //sendUdp(fdAccessServer,"POPRESP xxxx:127.0.0.1:58000 127.0.0.1:5900\n", 100, res);
                    printf("recebi um popreq \n");

                    //chamar a funçao que acha qual é o ponto de acesso
                    answerUdp(fdAccessServer, "POPRESP xxxy:127.0.0.1:58000 127.0.0.1:59000\n",100, (struct sockaddr *)&addr_udp);
                    printf("mandei um popresp \n");

                }
                


            }

            // When receives a message from up on the tree
            else if(fdUp != -1 && FD_ISSET(fdUp, &fd_sockets)){
                printf("i received something from TCP \n");

                readTcp(fdUp, bufferUp, PACKAGETCP);

                printf("o buffer tcp é: %s \n", bufferUp);
                if(root){
                        strcpy(bufferStream, "DATA ");
                        DecToHexStr(strlen(bufferUp)-1, strHexa);
                        strcat(bufferStream,strHexa);
                        strHexa[0] = '\0';
                        strcat(bufferStream,"\n");
                        strcat(bufferStream,bufferUp);
                        sendStreamToChilds(bufferStream,fdClients, tcpsessions);
                        printf("i received stream and from sourceServer\n");
                }
                else if(!root){
                    printf("i received from my dad \n");
                    n = sscanf(bufferUp,"%[^ ] %[^\n]\n %s",actionData,strHexa,contentStream);
                    if(n == 3 && !strcmp(actionData,"DATA")){
                            printf("%s \n",contentStream);
                            sendStreamToChilds(bufferStream,fdClients, tcpsessions);

                        }
                    
                }
            }  

            else if(fdTcpServer != -1 && FD_ISSET(fdTcpServer,&fd_sockets)){
                printf("received newClient \n");
                 if((newfd = accept(fdTcpServer, (struct sockaddr *) &addr_tcpServer, &addrlenTcpServer)) == -1) {
                         printf("Error while accepting new client\n");
                         exit(1);
                    }
                    if(clientsAvailable > 0){
                        if(insertFdClient(newfd,fdClients, tcpsessions)){
                            strcpy(bufferWelcome,"WE ");
                            strcat(bufferWelcome,streamId);
                            strcat(bufferWelcome,"\n");
                            strcat(bufferWelcome,"\0");

                            printf("bufferWelcome size é: %d \n",strlen(bufferWelcome));
                            writeTcp(newfd,bufferWelcome,strlen(bufferWelcome)-1);
                            clientsAvailable--;
                            bufferWelcome[0] = '\0';
                            printf("recebi um novo cliente  \n");
                        }
                        else{
                            writeTcp(fdUp,"RE",strlen("RE"));

                        }
                    }
                    else 
                        writeTcp(fdUp,"RE",strlen("RE"));


            } 

            
        }		
	}
	return 0;
}