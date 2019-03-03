#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "tcp.h"

void initTcp(struct addrinfo *hints_tcp){
    memset(hints_tcp, 0 ,sizeof(*hints_tcp));
    hints_tcp->ai_family=AF_INET;    
    hints_tcp->ai_socktype=SOCK_STREAM;   
    hints_tcp->ai_flags= AI_NUMERICSERV;
}

//as duas funçoes de baixo tem de se ver melhor de acordo com o nosso programa se nao e melhor retornar int e nao sei se faz sentido dar exit..
// se calhar fecha-se simplesmente o socket e continua-se mas depende do programa

void readTcp(int fd, char* buffer)
{
    char aux[128];  
    int n;         
    n=read(fd,aux,sizeof(aux));
    if(n==-1){
        printf("error reading in TCP \n");
        exit(1);
    }
    

    aux[n]='\0';
    strcat(buffer, aux);
}

void write_tcp(int fd, char *msg)
{
    int nSended;   
    int nBytes;     
    int nLeft;      

    nBytes=strlen(msg); 
    nLeft=nBytes;

    while(nLeft>0)
    {
        nSended=write(fd,msg,nLeft);
        if(nSended<=0){
            printf("error sending message \n");
        }

        nLeft-=nSended;
        msg+=nSended;
    }

    msg-=nBytes;
}