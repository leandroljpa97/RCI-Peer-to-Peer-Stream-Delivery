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
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PACKAGE_UDP 128
#define PACKAGE_TCP 128



//--------------------------------- ACCESS SERVER -----------------------------------------------------------



void  sendRandomAccessPoint(){
    int i;
    time_t t;
   
    /* Intializes random number generator */
    srand((unsigned) time(&t));

    while(1){
        i =  rand() % bestpops;
        if(accessPoints.fd[i] != 0){

            strcpy(ipAccessPoint,accessPoints.ip[i]);
            strcpy(portAccessPoint,accessPoints.port[i]);

            accessPoints.available ++ ;
            //this variable when is equal to bestpops so the root has to 
            //perform POP__QUERY! so that it is impossible to be greater than bestpops
            if(accessPoints.available > bestpops)
                accessPoints.available = bestpops;

            return;
        }
    
    }

    
}

void findAccessPoint(){

    if(accessPoints.available < bestpops){
        sendRandomAccessPoint();
        return;
    }

    else{
            char queryIdHex[] = "0000";

            // Converts queryID to hex format
            convertNumDoHex(queryIdHex, queryId);
            printf("%d in Hexadecimal = %s", queryId, queryIdHex);

        for(int i = 0; i < tcpsessions; i++){
            if(clients.fd[i] != 0)
                POP_QUERY(clients.fd[i],queryIdHex,bestpops);
        }

        queryId++;
        //verify if queryId is greater than 2^16 -1 ..
        if( queryId > 65534)
            queryId = 0;

        sendRandomAccessPoint();
    }

    

}

void insertAccessPoint(char _ip[], char _port[]){

    //when the list of accessPOints is full!!
    if(accessPoints.available == 0)
        return;

    for(int i = 0; i < bestpops; i++){

        if(accessPoints.fd[i] == 0){
            accessPoints.ip[i][0] = '\0';
            accessPoints.port[i][0] = '\0';
            strcpy(accessPoints.ip[i], _ip);
            strcpy(accessPoints.port[i], _port);

            accessPoints.fd[i] = 1;
	    accessPoints.available -- ;

            return;
        }
    }

    //if there isn't any position free but it is time to refresh the list of accessPOints!
    int j;
    time_t t;
   
    /* Intializes random number generator */
    srand((unsigned) time(&t));
    j =  rand() % bestpops;

    accessPoints.ip[j][0] = '\0';
    accessPoints.port[j][0] = '\0';
        

    strcpy(accessPoints.ip[j], _ip);
    strcpy(accessPoints.port[j], _port);
    
    accessPoints.fd[j] = 1;
    accessPoints.available -- ;

    return;
        

}


void insertAccessP_ifFree(char _newPopIp[], char _newPopPort[]){

    for(int i = 0; i < bestpops; i++){

        if(accessPoints.fd[i] == 0){
            accessPoints.ip[i][0] = '\0';
            accessPoints.port[i][0] = '\0';
            strcpy(accessPoints.ip[i], _newPopIp);
            strcpy(accessPoints.port[i], _newPopPort);

            accessPoints.fd[i] = 1;

            return;
        }
    }

}




//-----------------------------------------------------------------------------------------





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

    // Indicates how many bytes are already read from previous attemps
    int nAlreadyReceived = 0;

    // receive information from dad
    char bufferUp[PACKAGETCP];

    char bufferDown[BUFFER_SIZE];
    char action[BUFFER_SIZE];
    char sizeStream[BUFFER_SIZE];
    char idStream[BUFFER_SIZE];
    char actionChild[BUFFER_SIZE];
    char newPopPort[BUFFER_SIZE];
    char newPopIp[BUFFER_SIZE];
    char queryIdAux[BUFFER_SIZE];
    char bestpopsAux[BUFFER_SIZE];
    char avails[BUFFER_SIZE];



    // flag that is 1 if we didn't read everything 
    int checkReadUp = 0;


	// Read Input Arguments of the program and set the default variables
    int dumpSignal = readInputArguments(argc, argv, streamId, streamName, streamIp, streamPort, ipaddr, tport, 
                       				uport, rsaddr, rsport, &tcpsessions, &bestpops, &tsecs, 
                      				&dataStream, &debug);

    // Init clients struct
    initClientStructure();

    // Print all available streams
    if(dumpSignal == 1) {
    	printf("No streamId. We suggest streams \n");
    	DUMP();
        exit(1);
    }

    WHOISROOT(&root, &fdAccessServer, &fdUp);

    if(root)
        initAccessPoints();


    printf("fiz WHOISROOT\n");

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

        // Adds the file descriptor of the TCP to accept new clients
        if(fdDown != -1)
            addFd(&fd_sockets, &maxfd, fdDown);
        
        // Adds the file descriptor of the TCP to comm with clients
        for(int i =0; i < tcpsessions; i++)
            if(clients.fd[i] != 0)
                addFd(&fd_sockets, &maxfd, clients.fd[i]);

        // Time variables
        t1 = NULL;
        t2.tv_usec = 0;
        t2.tv_sec = TIMEOUT;
        t1 = &t2;

        // Monitor all the file descritors to check for new inputs
        counter = select(maxfd + 1, &fd_sockets, (fd_set*) NULL, (fd_set *) NULL, (struct timeval*) t1);     
      
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
                        findAccessPoint();
                        POPRESP(fdAccessServer, &addr, ipAccessPoint, portAccessPoint);
                    }
                }
            }
            // When receives a message from up on the tree
            else if(fdUp != -1 && FD_ISSET(fdUp, &fd_sockets)) {
                printf("i received something from TCP \n");

                if(root){
                    // Reads the stream from the source up to the maximum allowed size
                    // with n we determine how many bites were read, so we can prepare the buffer to send downstream
                    int n = readTcp(fdUp, bufferUp);

                    // Stream is left - the program needs to restart and try to reconect with the source again
                    if(n == 0) {
                        printf("Stream is gone \n");
                        close(fdUp);
                        fdUp = -1;
                    }

                    printf("o buffer tcp é: %s \n", bufferUp);
                    // Analise client list and send message
                    for (int i = 0; i < tcpsessions; ++i){
                        if(clients.fd[i] != 0) {
                            if(DATA(clients.fd[i], n, bufferUp) == 0) {
                                closeClient(clients.fd[i]);
                                printf("Child gone\n");
                            }
                        }
                    }
                    bufferUp[0] = '\0';
                }
                else if(!root){
                    printf("i received from my dad \n");

                    int n;

                    // if flag is 0 we have to do everything normal - read DA or WE 
                    if(!checkReadUp) {
                        printf("o checkReadUp tá a 0 \n");
                        n = readTcpNBytes(fdUp, action, TCP_MESSAGE_TYPE);
                        if(n == -1)  {
                            printf("Dad left\n");
                            // Chamar WHOISROOT
                        }
                        printf("action: %s\n", action);

                        if(strcmp(action, "DA ") == 0) {
                            // Reads the amount of bytes that will receive in 4 hex digits
                            n = readTcpNBytes(fdUp, sizeStream, TCP_MESSAGE_SIZE);
                            if(n == -1)  {
                                printf("Dad left\n");
                                // Chamar WHOISROOT
                            }
                            printf("sizeStream - size recebido: %d\n", n);

                            // Reads the content of the DATA package
                            n = readTcp(fdUp, bufferUp);
                            if(n == -1)  {
                                printf("Dad left\n");
                                // Chamar WHOISROOT
                            }
                            printf("bufferUp - size recebido: %dvs%d \n", n, (int) strtol(sizeStream, NULL, 16));
                            printf("bufferUp %s\n", bufferUp);

                            // Converts the sizeStreamOrId to decimal number, if the number 
                            //of bytes read is not the same that is indicate, we need to read again from the source
                            if(n != (int) strtol(sizeStream, NULL, 16)) {
                                checkReadUp = 1;
                                nAlreadyReceived = n;
                            }
                            else {
                                // Retransmit data to its clients
                                for (int i = 0; i < tcpsessions; ++i){
                                    if(clients.fd[i] != 0) {
                                        if(DATA(clients.fd[i], n, bufferUp) == 0) {
                                            closeClient(clients.fd[i]);
                                            printf("Child gone\n");
                                        }
                                    }
                                }
                                // Inputs the string terminator
                                bufferUp[0]= '\0';
                                idStream[0]= '\0';
                                sizeStream[0]= '\0';
                                nAlreadyReceived = 0;
                            }
                        }
                        else if(strcmp(action, "WE ") == 0) {
                            printf("I received a WELCOME\n");

                            n = readTcp(fdUp, idStream);
                            if(n == -1)  {
                                printf("Dad left\n");
                                // Chamar WHOISROOT
                            }
                            idStream[n-1] = '\0';
                            printf("idStream %s\n", idStream);


                            //in this case sizeId received is sizeStream
                            if(strcmp(idStream, streamId) >= 0) {
                                NEW_POP(fdUp);
                            }
                            
                            bufferUp[0]= '\0';
                            idStream[0] = '\0';

                        }

                        else if(!strcmp(action,"PQ ")){
                            n = readTcpNBytes(fdUp, queryIdAux, TCP_MESSAGE_SIZE);

                            if(n == -1)  {
                                printf("Dad left\n");
                                // Chamar WHOISROOT
                            }
                            n = readTcp(fdUp,bestpopsAux);

                            if(n == -1)  {
                                printf("Dad left\n");
                                // Chamar WHOISROOT
                            }

                            if(clients.available > 0){

                                POP_REPLY(fdUp, queryIdAux, ipaddr, tport, clients.available);

                                if(atoi(bestpopsAux) > clients.available){

                                    for(int j = 0; j < tcpsessions; j++){
                                        if(clients.fd[j] != 0)
                                            POP_QUERY(clients.fd[j],queryIdAux, atoi(bestpopsAux) - clients.available);
                                    }
                                }
                                insert_at_first(queryIdAux, atoi(bestpopsAux) - clients.available);

                            }



                        }

                        action[0] = '\0';
                        sizeStream[0] = '\0';
                        bestpopsAux[0] = '\0';
                        queryIdAux[0] = '\0';
                    }
                    // if checkReadUp = 1 concatennate bufferUp to received stream
                    else{
                        printf("o checkReadUp tá a 1 \n");
                        n = readTcp(fdUp, bufferUp);
                        if(n == -1) {
                            printf("Dad left\n");
                            // Chamar WHOISROOT
                        }

                        printf("bufferUp %s\n", bufferUp);

                        if((nAlreadyReceived + n) !=  (int) strtol(sizeStream, NULL, 16)) {
                            checkReadUp = 1;
                            nAlreadyReceived += n;
                        }
                        else {
                            // Retransmit data to its clients
                            for (int i = 0; i < tcpsessions; ++i){
                                if(clients.fd[i] != 0) {
                                    if(DATA(clients.fd[i], nAlreadyReceived + n, bufferUp) == 0) {
                                        closeClient(clients.fd[i]);
                                        printf("Child gone\n");
                                    }
                                }
                            }
                            nAlreadyReceived = 0;
                            checkReadUp = 0;
                            sizeStream[0] = '\0';
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

                if(clients.available > 0){

                    if((newfd = accept(fdDown, (struct sockaddr *) &addr_tcpServer, &addrlenTcpServer)) == -1) {
                        printf("Error while accepting new client\n");
                        exit(1);
                    }
                    
                    if(insertFdClient(newfd, &clients)) {
                        // Sends the welcome message to the new client
                        if(!WELCOME(newfd)) {
                            deleteFdClient(newfd, &clients);
                        }
                        printf("mandei um welcome \n");
                    }
                    else{
                        writeTcp(fdUp,"RE",strlen("RE"));

                    }
                }
                else {
                    writeTcp(fdUp,"RE",strlen("RE"));
                }
            } 
            else if(clients.available < tcpsessions){
                for(int i =0; i < tcpsessions; i++) {
                    if(clients.fd[i] != 0 && FD_ISSET(clients.fd[i],&fd_sockets)){
                        printf("recebi algo do meu filho com o id=%d \n",clients.fd[i]);
                        int n; 

                        n = readTcpNBytes(clients.fd[i], actionChild, TCP_MESSAGE_TYPE);
                        if(n == -1)  {
                            printf("Child left\n");
                            // Chamar WHOISROOT
                        }
                        printf("actionChild: %s\n", actionChild);

                        if(strcmp(actionChild, "NP ") == 0) {
                            // Reads the amount of bytes that will receive in 4 hex digits
                            n = readTcp(clients.fd[i],bufferDown);
                            if(n == -1){
                                printf("Child left\n");
                                // Chamar WHOISROOT
                            }

                            n = sscanf(bufferDown,"%[^:]:%[^\n]\n",newPopIp, newPopPort);
                        
                            addClient(clients.fd[i], newPopIp, newPopPort);

                            if(root)
                                insertAccessP_ifFree(newPopIp, newPopPort);

                            printf(" clients.ip[i]:%s\n", clients.ip[i]);
                            printf(" clients.port[i]:%s\n", clients.port[i]);

                        }

                        else if(!strcmp(actionChild,"PR ")){
                            int availsSend;
                            n = readTcp(clients.fd[i], bufferDown);
                            if(n == -1){
                                printf("Child left\n");
                                // Chamar WHOISROOT
                            }

                            n = sscanf(bufferDown,"%[^ ] %[^:]:%[^ ] %[^\n]\n",queryIdAux , newPopIp, newPopPort, avails);

                            if(root)
                                insertAccessPoint(newPopIp, newPopPort);

                            else{

                                n = getLeft(queryIdAux);
                                if(atoi(avails) <= n)
                                    availsSend = atoi(avails);
                                else 
                                    availsSend = n;

                                if(availsSend != 0){
                                    POP_REPLY(fdUp,queryIdAux,newPopIp, newPopPort, availsSend);
                                    for(int j = 0; j < availsSend;j++)
                                        decrementItem(queryIdAux);
                                }

                                
                            }
                          
                        }


                        bufferDown[0] = '\0';
                        actionChild[0] = '\0';
                        newPopPort[0] = '\0';
                        newPopIp[0] = '\0';
                        avails[0] = '\0';
                        queryIdAux[0] = '\0';
                    }
                }
            }
 
        }

    }

	return 0;
}




