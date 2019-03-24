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

    // indice to say who was the last child that i've redirect to
    int lastChild = -1;

    // receive information from dad
    char bufferUp[PACKAGETCP];
    memset(bufferUp, '\0', PACKAGETCP);

    char sizeStream[BUFFER_SIZE];
    char idStream[BUFFER_SIZE];
    char newPopPort[BUFFER_SIZE];
    char newPopIp[BUFFER_SIZE];
    char queryIdAux[BUFFER_SIZE];
    char bestpopsAux[BUFFER_SIZE];
    char avails[BUFFER_SIZE];

    // Resets the number of AP availables on the list
    numberOfAP = 0;

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
        for(int i =0; i < tcpsessions; i++) {
            if(clients.fd[i] != 0) {
                addFd(&fd_sockets, &maxfd, clients.fd[i]);
            }
        }

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
            if(FD_ISSET(0, &fd_sockets)) {
                char userInput[BUFFER_SIZE];
                // Clean the buffers
		    	memset(userInput, 0, sizeof(userInput));
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
                struct sockaddr_in addr;

                receiveUdp(fdAccessServer, bufferAccessServer, BUFFER_SIZE, &addr);

                if(strstr(bufferAccessServer, "POPREQ") != NULL) {
                    printf("RECEIVED A POPREQ\n");
                    // If root has available connection, allow connection to itself
                    if(clients.available > 0) {
                        // Sends POPRESP with IP and Port to connect to itself
                        POPRESP(fdAccessServer, &addr, ipaddr, tport);
                    }
                    else {
                        int findAP; 
                        // Gets an IP and Port from the list of AP and responds to the client
                        if((findAP = getAccessPoint(ipAccessPoint, portAccessPoint)) != -1){
                            POPRESP(fdAccessServer, &addr, ipAccessPoint, portAccessPoint);
                            printf("tnh la access points \n");
                        }

                        // Needs to find new AP to the list
                        if(findAP == 1) {
                            queryId++;
                            for(int i = 0; i < tcpsessions; i++)
                                POP_QUERYroot(clients.fd[i], queryId, bestpops);
                        }
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
                    if(n <= 0) {
                        printf("Stream is gone \n");
                        close(fdUp);
                        fdUp = -1;
                    }

                    printf("o buffer tcp é: %s \n", bufferUp);
                    // Analise client list and send message
                    for (int i = 0; i < tcpsessions; ++i){
                        if(clients.fd[i] != 0) {
                            if(DATA(clients.fd[i], n, bufferUp) == 0) {
                                removeChild(i);
                                printf("Child gone\n");
                            }
                        }
                    }
                    bufferUp[0] = '\0';
                }
                else if(!root){
                    printf("i received from my dad \n");

                    // Reads the message from its dad
                    int n = readTcp(fdUp, bufferUp);
                    if(n <= 0)  {
                        close(fdUp);
                        fdUp = -1;

                        for(int j = 0; j < tcpsessions; j++)
                            if(clients.fd[j] != 0)
                                BROKEN_STREAM(clients.fd[j]);

                        printf("Dad left\n");
                        WHOISROOT(&root,&fdAccessServer,&fdUp);

                        // Chamar WHOISROOT
                        continue;
                    }

                    printf("bufferUp %s\n", bufferUp);

                    int newAction = 1;
                    while(newAction == 1) {
                        printf("action: %c%c\n", bufferUp[0], bufferUp[1]);

                        if(bufferUp[0] == 'D' && bufferUp[1] == 'A') {
                            printf("I received DATA\n");  
                            printf("bufferUp %s \n", bufferUp);     
                            // Copies the amount of bytes that will receive in 4 hex digits
                            strncpy(sizeStream, &bufferUp[3], TCP_MESSAGE_SIZE);

                            // Checks if it received the complete DA message
                            if((int) strtol(sizeStream, NULL, 16) + 8 <= n) {
                                // Retransmit data to its clients
                                for (int i = 0; i < tcpsessions; ++i) {
                                    if(clients.fd[i] != 0) {
                                        if(DATA(clients.fd[i], (int) strtol(sizeStream, NULL, 16), &bufferUp[8]) == 0) {
                                            removeChild(i);
                                            printf("Child gone\n");
                                        }
                                    }
                                }
                                // Checks if more messages are on the buffer
                                if(((int) strtol(sizeStream, NULL, 16) + 9 < PACKAGE_TCP) && (bufferUp[(int) strtol(sizeStream, NULL, 16) + 9] != '\0')) {
                                    // Copies the buffer to the beggining
                                    strcpy(bufferUp, &bufferUp[(int) strtol(sizeStream, NULL, 16) + 9]);

                                    // Indicates that the size received is the one received minus the messaged that is already processed
                                    n -= (int) strtol(sizeStream, NULL, 16) + 8;
                                }
                                else {
                                    // Clears the bufferUp string, since it's jobs is done
                                    memset(bufferUp, '\0', PACKAGE_TCP);
                                    newAction = 0;
                                } 

                                // Clears the size stream since DATA is done
                                memset(sizeStream, '\0', BUFFER_SIZE);
                            }
                            // The data is not complete                            
                            else {
                                newAction = 0;
                            }
                        }
                        else if(bufferUp[0] == 'W' && bufferUp[1] == 'E') {
                            printf("I received a WELCOME\n");
                            int newLine = 0;
                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0){
                                sscanf(&bufferUp[3], "%[^\n]\n", idStream);

                                //in this case sizeId received is sizeStream
                                if(strcmp(idStream, streamId) == 0) {
                                    NEW_POP(fdUp);
                                }
                                // Clears the idStram string, since it's jobs is done
                                memset(idStream, '\0', PACKAGE_TCP);

                                // checks if there is another message
                                if(bufferUp[newLine + 1] != '\0') {
                                    // Copies the buffer to the beggining
                                    strcpy(bufferUp, &bufferUp[newLine + 1]);
                                }
                                // There's no more messages
                                else {
                                    newAction = 0;
                                    memset(bufferUp, '\0', PACKAGE_TCP);
                                }
                            }
                            // The data is not complete
                            else {
                                newAction = 0;
                            }
                        }
                        else if(bufferUp[0] == 'P' && bufferUp[1] == 'Q'){
                            printf("I received a POP_QUERYclients\n");
                            int newLine = 0;
                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0) {
                                // checks if both informations are contained there
                                if(sscanf(&bufferUp[3], "%[^ ] %[^\n]\n", queryIdAux, bestpopsAux) == 2) {
                                    printf("queryIdAux: %s\nbestpopsAux: %s\n", queryIdAux, bestpopsAux);

                                    // Proccess the POP_QUERY
                                    if(clients.available > 0) {
                                        // If the iam has available tcp but not enough to cover the request.
                                        if(atoi(bestpopsAux) > clients.available){
                                            POP_REPLY(fdUp, queryIdAux, ipaddr, tport, clients.available);
                                            // Run the list of clients to send the message to search for more bestpops left
                                            for(int j = 0; j < tcpsessions; j++){
                                                if(clients.fd[j] != 0)
                                                    POP_QUERYclients(clients.fd[j], queryIdAux, atoi(bestpopsAux) - clients.available);
                                            }
                                            // Insert the pending request bestpops
                                            insertQueryID(queryIdAux, atoi(bestpopsAux) - clients.available);
                                        }
                                        else {
                                            POP_REPLY(fdUp, queryIdAux, ipaddr, tport, atoi(bestpopsAux));
                                        }
                                    }
                                    // Clears the idStram string, since it's jobs is done
                                    memset(queryIdAux, '\0', BUFFER_SIZE);
                                    memset(bestpopsAux, '\0', BUFFER_SIZE);
                                }

                                // checks if there is another message
                                if(bufferUp[newLine + 1] != '\0') {
                                    // Copies the buffer to the beggining
                                    strcpy(bufferUp, &bufferUp[newLine + 1]);
                                }
                                // There's no more messages
                                else {
                                    newAction = 0;
                                    memset(bufferUp, '\0', PACKAGE_TCP);
                                }
                            }
                            // The data is not complete
                            else {
                                newAction = 0;
                            }
                        }

                        else if(bufferUp[0] == 'B' && bufferUp[1] == 'S'){
                            int newLine = 0;
                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0) {
                               
                               for(int j = 0; j < tcpsessions; j++)
                                    if(clients.fd[j] != 0)
                                        BROKEN_STREAM(clients.fd[j]);


                                // checks if there is another message
                                if(bufferUp[newLine + 1] != '\0') {
                                    // Copies the buffer to the beggining
                                    strcpy(bufferUp, &bufferUp[newLine + 1]);
                                }
                                // There's no more messages
                                else {
                                    newAction = 0;
                                    memset(bufferUp, '\0', PACKAGE_TCP);
                                }
                            }
                            // The data is not complete
                            else {
                                newAction = 0;
                            }
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

                if((newfd = accept(fdDown, (struct sockaddr *) &addr_tcpServer, &addrlenTcpServer)) == -1) {
                    printf("Error while accepting new client\n");
                    exit(1);
                }
                if(clients.available > 0){
                    if(insertFdClient(newfd)) {
                        // Sends the welcome message to the new client
                        if(!WELCOME(newfd)) {
                            deleteFdClient(newfd);
                        }
                        printf("mandei um welcome \n");
                    }
                    else{
                        int n = getIndexChild(lastChild+1);
                        lastChild = n;
                        REDIRECT(newfd,clients.ip[lastChild], clients.port[lastChild]);
                        close(newfd);
                    }
                }
                else {
                    int n = getIndexChild(lastChild+1);
                    lastChild = n;
                    REDIRECT(newfd,clients.ip[lastChild], clients.port[lastChild]);
                    close(newfd);
                }
            } 
            // When receives messages from its clients
            else if(clients.available < tcpsessions){
                for(int i = 0; i < tcpsessions; i++) {
                    printf("clients.fd[%d] %d\n", i, clients.fd[i]);
                    if(clients.fd[i] != 0 && FD_ISSET(clients.fd[i],&fd_sockets)){
                        printf("recebi algo do meu filho com o id=%d \n",clients.fd[i]);
                        int n = readTcp(clients.fd[i], clients.buffer[i]);
                        if(n <= 0)  {
                            printf("Child left\n");
                            removeChild(i);
                            continue;

                        }

                        printf("clients.buffer[%d]: %s\n", i, clients.buffer[i]);

                        int newAction = 1;
                        while(newAction == 1) {
                            // Receives a NEW_POP
                            if(clients.buffer[i][0] == 'N' && clients.buffer[i][1] == 'P') {
                                printf("Received a NEW_POP\n");

                                int newLine = 0;

                                // Found a complete message
                                if((newLine = findsNewLine(clients.buffer[i], PACKAGE_TCP)) >= 0){
                                    // checks if both informations are contained there
                                    if(sscanf(&clients.buffer[i][3], "%[^:]:%[^\n]\n",newPopIp, newPopPort) == 2) {
                                        printf("newPopIp: %s\newPopPort: %s\n", newPopIp, newPopPort);

                                        // Adds the new client to the list of clients
                                        addClient(clients.fd[i], newPopIp, newPopPort);

                                        // When its root, adds the new client to the list of AP - since it doesn't know how many tcpsessions it has, insert has 1 (default)
                                        if(root)
                                            insertAccessPoint(newPopIp, newPopPort, 1);

                                        printf(" clients.ip[i]:%s\n", clients.ip[i]);
                                        printf(" clients.port[i]:%s\n", clients.port[i]);

                                        // Clears the idStram string, since it's jobs is done
                                        memset(newPopIp, '\0', BUFFER_SIZE);
                                        memset(newPopPort, '\0', BUFFER_SIZE);
                                    }
                                    // checks if there is another message
                                    if(clients.buffer[i][newLine + 1] != '\0') {
                                        // Copies the buffer to the beggining
                                        strcpy(clients.buffer[i], &clients.buffer[i][newLine + 1]);
                                    }
                                    // There's no more messages
                                    else {
                                        newAction = 0;
                                        memset(clients.buffer[i], '\0', PACKAGE_TCP);
                                    }
                                }
                                // The data is not complete
                                else {
                                    newAction = 0;
                                }
                            }
                            // Receives a POP-REPLY
                            else if(clients.buffer[i][0] == 'P' && clients.buffer[i][1] == 'R'){
                                printf("Received a POP_REPLY\n");
                                printf("recebi um %s \n", clients.buffer[i]);

                                int newLine = 0;
                                // Found a complete message
                                if((newLine = findsNewLine(clients.buffer[i], PACKAGE_TCP)) >= 0){
                                    // checks if both informations are contained there
                                    printf("sim entrei \n");
                                    if(sscanf(&clients.buffer[i][3],"%[^ ] %[^:]:%[^ ] %[^\n]\n", queryIdAux, newPopIp, newPopPort, avails) == 4) {
                                        int availsSend;

                                        printf("queryIdAux2 %s \n", queryIdAux);
                                            printf("newPopIp2 %s \n", newPopIp);
                                            printf("Port2 %s \n", queryIdAux);



                                        // When it's root, insert the number of tcp sessions that the iamroot is able to have
                                        if(root){
                                            // Finds how many bestpops are still to find to that queryID
                                            n = getLeftQueryID(queryIdAux);
                                            // if the client has more tcp sessions than its needed - inputs only the necessary ones
                                            if(atoi(avails) <= n)
                                                availsSend = atoi(avails);
                                            else 
                                                availsSend = n;

                                            if(availsSend != 0){
                                                // checks if the AP is already on the list or if the current AP bestpops on the list is smaller than the new receive value
                                                if(isAPontTheList(newPopIp, newPopPort, availsSend) == 1) {
                                                    // Inputs the new access points
                                                    insertAccessPoint(newPopIp, newPopPort, availsSend);
                                                    for(int j = 0; j < availsSend; j++)
                                                        decrementQueryID(queryIdAux);
                                                }
                                            }
                                        }
                                        else{
                                            printf("queryIdAux1 %s \n", queryIdAux);
                                            printf("newPopIp1 %s \n", newPopIp);
                                            printf("Port1 %s \n", queryIdAux);


                                            // Finds how many bestpops are still to find to that queryID
                                            n = getLeftQueryID(queryIdAux);
                                            if(atoi(avails) <= n)
                                                availsSend = atoi(avails);
                                            else 
                                                availsSend = n;

                                            if(availsSend != 0){
                                                POP_REPLY(fdUp,queryIdAux,newPopIp, newPopPort, availsSend);
                                                for(int j = 0; j < availsSend; j++)
                                                    decrementQueryID(queryIdAux);
                                            }
                                        }
                                        memset(newPopPort, '\0', BUFFER_SIZE);
                                        memset(avails, '\0', BUFFER_SIZE);
                                        memset(newPopIp, '\0', BUFFER_SIZE);
                                        memset(queryIdAux, '\0', BUFFER_SIZE);
                                    }
                                    // checks if there is another message
                                    if(clients.buffer[i][newLine + 1] != '\0') {
                                        // Copies the buffer to the beggining
                                        strcpy(clients.buffer[i], &clients.buffer[i][newLine + 1]);
                                    }
                                    // There's no more messages
                                    else {
                                        printf("theres no more messages \n");
                                        newAction = 0;
                                        memset(clients.buffer[i], '\0', PACKAGE_TCP);
                                    }
                                }
                                // The data is not complete
                                else {
                                    printf("oii? \n");
                                    newAction = 0;
                                }
                            }
                        } 
                    }
                }
            }
            else {
                printf("I received something, but didn't read the message\n");
            }
 
        }

    }

	return 0;
}