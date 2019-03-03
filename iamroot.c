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
#define MAX_LENGTH 50

#define TIMEOUT 30

#define ROOTSERVER 1
#define FIND_ROOT 2
#define FIND_DAD 3
#define NORMAL 4


/*
 * initFdSockets: sets an empty mask of file descriptors to be controlled by select
 */
void initFdSockets(fd_set * _fd_sockets, int* _maxfd){
	FD_ZERO(_fd_sockets);
	FD_SET(0,_fd_sockets);
	_maxfd = 0;

}

/*
 * addFd: add a new file descriptor to be controlled by select
 */
void addFd(fd_set * _fd_sockets, int* _maxfd, int _fd){
    FD_SET(_fd, _fd_sockets);
    *_maxfd = _fd;
}



void dump(int _fd, struct addrinfo *_res){
    sendUdp(_fd,"DUMP\n",_res);
    printf("sai de dentro do dump \n");
}
void Remove(int _fd, struct addrinfo *_res, char _streamId[]){
    char buffer[150];
    strcpy(buffer,"REMOVE ");
    strcat(buffer,_streamId);
    sendUdp(_fd,"DUMP\n",_res);
}



void interpRootServerMsg( int _fd_udp, struct addrinfo *_res, char _data[]){
    char content[50];
    int flag = sscanf(_data, "%s", content);
            printf(" a mensagem vinda do stdi  é: %s \n",content);
            if (flag<0){
                printf("Error reading stdin. Exit(0) the program\n");
                if(_fd_udp!=-1) close(_fd_udp);
                exit(0);
            }
            else{
                if(!strcmp(content,"streams")){
                    dump(_fd_udp,_res);

                }
                else if(!strcmp(content,"status")){
                    printf("presses status \n");

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

                }
                else 
                    printf("wrong command, try again \n");
            }

}


        

int main(int argc, char* argv[]){
	//files descriptor	
	//fd_up is to TCP communication with the source (if this node is root) or with the dad						
	int fd=-1,fd_udp=-1,fd_up=-1, addrlen_udp, addrlen_tcp,n;
	fd_set fd_sockets;	
	//variables to use in timeout of select							
	struct timeval* t1 = NULL;
	struct timeval t2;
	char ip_root_server[20] = "193.136.138.142";
	char port_root_server[17] = "59000";
	char ipaddr[] = "000.000.000.000";
    char tport[] = "58000";
    char uport[] = "58000";
    int tcpsessions = 1;
    int bestpops = 1;
    int tsecs = 5;
    int dataStream = 1;
    int debug = 0;

    char streamId[64];
    char streamName[45];
    char sourceIp[20] = "193.136.138.142";
    char sourcePort[16]="58011";
    //confirmar se nao há um default para meter
	//valores um pouco a toa...			
	char action[50], data[100],content[65],buffer[128], buffer_tcp[128], ipaddr_aux[40] ,uport_aux[40];
	int counter= 0, maxfd = 0;
	char *aux;
	int flag = 0;
	int status= -1;
    int root =0;


	//SOCKET UDP and TCP ! 
    struct addrinfo hints_tcp, *res_tcp, hints_udp;
    struct sockaddr_in addr_tcp, addr_udp;    

    // Read Input Arguments of the program and set the default variables
    readInputArguments(argc, argv, ipaddr,tport,uport,ip_root_server, port_root_server, &tcpsessions, &bestpops,
                     &tsecs,streamId,streamName,sourceIp,sourcePort,&dataStream,&debug);
    
    // Initiate UPD and TCP strucuture details
    initUdp(&hints_udp);
    initTcp(&hints_tcp);

    // Initiates UPD Sockets for datagram communication
    struct addrinfo  *res = createUdpSocket(&fd_udp,ip_root_server, port_root_server,&hints_udp);

    fd = fd_udp;

    // Establishes the status as a root stream
    status = ROOTSERVER;

    // Communicates with the root server to check to how to connect with
    whoIsRoot(fd_udp,res, streamId, ipaddr,uport);
    printf("fiz o whoIsRoot \n");
    //dump(fd_udp, res);


	while(1){	
		// Não estou a enteder para que server estes bufffers so com terminadores de string
		buffer[0]='\0';
        buffer_tcp[0]='\0';
		action[0]='\0';
		data[0]='\0';
		content[0]='\0';

		// Inits the mask of file descriptor
        initFdSockets(&fd_sockets, &maxfd);

        // Adds the file descriptor for the sdandart input - keyboard
        addFd(&fd_sockets, &maxfd,fd_udp);

        // Adds the file descriptor of the TCP to comm with root
        if(fd_up!=-1)
            addFd(&fd_sockets, &maxfd,fd_up);



		t1=NULL;
		//mudar estes valores e meter define - 30 segundos!
		t2.tv_usec = 0;
		t2.tv_sec = TIMEOUT;
		t1=&t2;

        if(status == FIND_ROOT){
            //establish communication with sourceServer, with ip and port obtained in streamId
            if(root){
                printf("VOU MORRER AQUI --------------------------------------------------------------------------------------- \n ");
                //o que se vai deixar é o de cima, mas meti o de baixo com o ncat, por isso a testares mete com o teu server!!
                //n = getaddrinfo(sourceIp, sourcePort,&hints_tcp, &res_tcp);
                n = getaddrinfo("192.168.1.7","58100",&hints_tcp, &res_tcp);
                if(n!=0) {
                    printf("error getaddrinfo in TCP source server \n");
                    exit(1);
                }

                fd_up = socket(res_tcp->ai_family, res_tcp->ai_socktype, res_tcp->ai_protocol);
                if(fd_up==-1) {
                    printf("error creating TCP socket TCP to source server!! \n ");
                    exit(1);
                }

                n = connect(fd_up, res_tcp->ai_addr, res_tcp->ai_addrlen);
                if(fd_up==-1) {
                    printf("error in connect with TCP socket TCP in source!! \n ");
                    exit(1);
                } 
                fd=fd_up;
                status= NORMAL; 
                printf("o fd depois do tcp é : %d \n", fd);

            }
            // if the node isn't root, he establish a connection with the root 
            else if(!root){
                //igual ao de cima!!! ->
                //n = getaddrinfo(ipaddr_aux ,uport_aux  ,&hints_tcp, &res_tcp);
                n = getaddrinfo("192.168.1.7","58100",&hints_tcp, &res_tcp);

                if(n!=0) {
                    printf("error getaddrinfo in TCP source server \n");
                    exit(1);
                }

                fd_up = socket(res_tcp->ai_family, res_tcp->ai_socktype, res_tcp->ai_protocol);
                if(fd_up==-1) {
                    printf("error creating TCP socket TCP to source server!! \n ");
                    exit(1);
                 }

                n = connect(fd_up, res_tcp->ai_addr, res_tcp->ai_addrlen);
                if(fd_up==-1) {
                    printf("error in connect with TCP socket TCP in source!! \n "); 
                    exit(1);
                } 
                fd=fd_up;
                status=FIND_DAD;
            }
        }

        // Monitor all the file descritors to check for new inputs
		counter = select(maxfd+1, &fd_sockets, (fd_set*)NULL, (fd_set *)NULL, (struct timeval*)t1);		
		
		if(counter < 0){
			perror("Error in select");
			if (fd!=-1) 
				close(fd);
			exit(0);
		}

		//go out of the select because of timeout
		if(!counter){
			printf("timeout!! \n");
			//nao sei o que fazer
		}

		 // fd=0 is stdin
		if(FD_ISSET(0, &fd_sockets)){
			aux = fgets(data, 100, stdin);

			if (aux == NULL)
				printf("Nothing to read in stdin.\n");
            else 
                interpRootServerMsg(fd_udp,res,data);  
		
		}
        
		else if(FD_ISSET(fd_udp, &fd_sockets)){
			printf("recebi algo em udp yeaaaaaah \n");
	        receiveUdp(fd_udp, buffer, &addr_udp);
	        n = sscanf(buffer, "%[^ ] %[^ ] %[^:]:%[^\n]\n", action,content ,ipaddr_aux ,uport_aux );
	        printf("a action é: %s \n",action);

	        if((!strcmp(action,"URROOT"))&& status==ROOTSERVER){
	        	root=1;
	        	status=FIND_ROOT;

	        }

	        else if(!strcmp(action,"ROOTIS")){
	        	//aqui o content ´e a stream id
	        	printf("a streamID é: %s \n",content);
	        	printf("o ip da root é %s \n",ipaddr_aux);
	        	printf("o porto da root é %s \n",uport_aux);
	        	if(status==ROOTSERVER){
	        		status=FIND_ROOT;
	        		root=0;
	        	}

	        }
            else if(!strcmp(action,"STREAMS"))
            {
                printf("as streams sao %s \n",content);
            }
	        else if(!strcmp(action,"ERROR")){
	        	//aqui a content é a mensagem de erro!!
	        	printf("a mensagem de erro é %s \n",content);
	        }


        	printf("buffer: %s \n",buffer); 

		}

		else if(FD_ISSET(fd_up, &fd_sockets)){
			printf("i received something from TCP \n");
            readTcp(fd_up,buffer_tcp);
            printf("o buffer tcp é: %s \n",buffer_tcp);
            if(root){
                printf("i received stream and from sourceServer \n");
            }
            else if(!root){
                printf("i received from my dad \n");
            }
		} 
	}
	return 0;
}