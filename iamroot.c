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
#include <ctype.h> 

#define PACKAGE_UDP 128
#define PACKAGE_TCP 128






void interpRootServerMsg(char _data[]) {
    printf("data: %s \n",_data);
    char contentAux[100], content[100];
    int flag = sscanf(_data, "%[^\n]\n", contentAux);
    printf("contAux %s \n",contentAux);

    for(int i = 0; i< strlen(contentAux) ; i++)
        content[i] = tolower(contentAux[i]);
    content[strlen(contentAux)] = '\0';
    printf(".... %s \n",content);

    printf(" a mensagem vinda do stdi  é: %s \n",content);
    if (flag < 0){
        printf("Error reading stdin. Exit(0) the program\n");   
    }
    else{
        if(!strcmp(content,"streams")) {
            DUMP();
        }
        else if(!strcmp(content,"status")){
            printf("presses status \n");
            printf("stream identification: %s \n", streamId);

            if(broken)
                printf("Stream broken \n");
            else 
                printf("Stream not broken \n");

            if(root){
                printf("Is this app root?: YES \n");
                printf("Access Server IP:%s and Port: %s \n", ipaddr, uport);
            }
            else{
                printf("Is this app root?: NO \n");
                printf("My dad IP: %s and Port: %s \n", availableIAmRootIP, availableIAmRootPort);
            }
            printf("Access Point IP: %s and Port: %s \n", ipaddr, tport);
            printf("Access Points of downstream points - Ip:Port \n");
            for(int j = 0; j < tcpsessions; j++){
                if(clients.fd[j] != 0)
                    printf(" %s:%s \n", clients.ip[j], clients.port[j]);

            }


        }
        else if(!strcmp(content,"display on")){
            printf("pressed display on \n");
            dataStream = 1;

        }
        else if(!strcmp(content,"display off")){
            printf("pressed display off \n");
            dataStream = 0;

        }
        else if(!strcmp(content,"format ascii")){
            printf("pressed ascii \n");
            ascii = 1;
        }
        else if(!strcmp(content,"format hex")){
            printf("pressed format hex \n");
            ascii = 0;

        }
        else if(!strcmp(content,"debug on")){
            printf("pressed debug on \n");
            debug = 1;

        }
        else if(!strcmp(content,"debug off")){
            printf("pressed debug off \n");
            debug = 0;

        }
        else if(!strcmp(content,"tree")){

            if(root){
                printf("pressed tree \n");
                printf("%s\n", streamId);
                printf("%s:%s (%d", ipaddr, tport, tcpsessions);
                for (int i = 0; i < tcpsessions; ++i) {
                    if(clients.fd[i] != 0) {
                        printf(" %s:%s", clients.ip[i], clients.port[i]);
                        TREE_QUERY(clients.fd[i], clients.ip[i], clients.port[i]);
                    }
                }
                printf(")\n");
             }
             else
                printf("you are not Root. YOu can not do TREE_QUERY \n");

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
    
    char bufferHex[PACKAGETCP];
    memset(bufferHex, '\0', PACKAGETCP);

    char sizeStream[BUFFER_SIZE];
    char idStream[BUFFER_SIZE];
    char newPopPort[BUFFER_SIZE];
    char newPopIp[BUFFER_SIZE];
    char queryIdAux[BUFFER_SIZE];
    char bestpopsAux[BUFFER_SIZE];
    char avails[BUFFER_SIZE];
    char TQip[BUFFER_SIZE];
    char TQport[BUFFER_SIZE];
    char TRip[BUFFER_SIZE];
    char TRport[BUFFER_SIZE];
    char TRtcpsessions[BUFFER_SIZE];

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

    // Time variables
    t1 = NULL;
    t2.tv_usec = 0;
    t2.tv_sec = tsecs;
    t1 = &t2;

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

        if(root)
            printListCLient();

        

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
            // Time variables
            t1 = NULL;
            t2.tv_usec = 0;
            t2.tv_sec = tsecs;
            t1 = &t2;

            if(root) {
                if(WHOISROOTwithoutResponse() == 0)
                    printf("Unable to make WHO IS ROOT periódico\n");
                
                // Run the list of clients to send the message to search for more bestpops left
                queryId++;
                /*for(int j = 0; j < tcpsessions; j++){
                    if(clients.fd[j] != 0)
                        if(!POP_QUERYroot(clients.fd[j], queryId, bestpops))
                            removeChild(j);
                } 
                // Insert the pending request bestpops
                insertQueryIDroot(queryId, bestpops); */
            }
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
                    printf("clients.available na root %d\n", clients.available);
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
                            insertQueryIDroot(queryId, bestpops);
                            for(int i = 0; i < tcpsessions; i++)
                                if(!POP_QUERYroot(clients.fd[i], queryId, bestpops))
                                    removeChild(i);
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
                        
                        REMOVE();
                        DadLeft(&root,&fdAccessServer,&fdUp);
                        continue;

                    }

                    if(dataStream){
                        if(ascii){
                            printf("o buffer tcp é: %s \n", bufferUp);
                        }
                        else {
                            AsciiToHex(bufferUp,bufferHex);
                            printf("o buffer tcp é: %s \n", bufferHex);

                        }

                    }
                    // Analise client list and send message
                    for (int i = 0; i < tcpsessions; ++i){
                        if(clients.fd[i] != 0) {
                            if(!DATA(clients.fd[i], n, bufferUp)) {
                                removeChild(i);
                                printf("Child gone\n");
                            }
                        }
                    }
                    bufferUp[0] = '\0';
                    bufferHex[0] = '\0';
                }
                else if(!root){
                    printf("i received from my dad \n");

                    // Reads the message from its dad
                    int n = readTcp(fdUp, bufferUp);
                    if(n <= 0)  {
                        DadLeft(&root,&fdAccessServer,&fdUp);
                        continue;
                    }


                    int newAction = 1;
                    while(newAction == 1) {
                        printf("action: %c%c\n", bufferUp[0], bufferUp[1]);

                        if(bufferUp[0] == 'D' && bufferUp[1] == 'A') {
                            printf("I received DATA\n"); 

                            if(dataStream) {
                                if(ascii){
                                    printf("o buffer tcp é: %s \n", bufferUp);
                                }
                                else {
                                    AsciiToHex(bufferUp,bufferHex);
                                    printf("o buffer tcp é: %s \n", bufferHex);

                                }
                            }
                            // Copies the amount of bytes that will receive in 4 hex digits
                            strncpy(sizeStream, &bufferUp[3], TCP_MESSAGE_SIZE);

                            // Checks if it received the complete DA message
                            if((int) strtol(sizeStream, NULL, 16) + 8 <= n) {
                                // Retransmit data to its clients
                                for (int i = 0; i < tcpsessions; ++i) {
                                    if(clients.fd[i] != 0) {
                                        if(!DATA(clients.fd[i], (int) strtol(sizeStream, NULL, 16), &bufferUp[8])) {
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
                                    memset(bufferHex, '\0', PACKAGE_TCP);

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
                                    if(!NEW_POP(fdUp)){
                                       DadLeft(&root, &fdAccessServer, &fdUp);
                                        continue;

                                    }
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

                                    printf("clients.available !! %d\n", clients.available);


                                    // Proccess the POP_QUERY
                                    if(clients.available > 0) {
                                        // If the iam has available tcp but not enough to cover the request.
                                        if(atoi(bestpopsAux) > clients.available){
                                            if(!POP_REPLY(fdUp, queryIdAux, ipaddr, tport, clients.available)){
                                                deleteQueryID(queryIdAux);
                                                DadLeft(&root, &fdAccessServer, &fdUp);
                                                 continue;

                                            }
                                            else{
                                                // Run the list of clients to send the message to search for more bestpops left
                                                for(int j = 0; j < tcpsessions; j++){
                                                    if(clients.fd[j] != 0)
                                                        if(!POP_QUERYclients(clients.fd[j], queryIdAux, atoi(bestpopsAux) - clients.available))
                                                            removeChild(j);
                                                }
                                                // Insert the pending request bestpops
                                                insertQueryID(queryIdAux, atoi(bestpopsAux) - clients.available);
                                            }
                                        }
                                        else {

                                            if(!POP_REPLY(fdUp, queryIdAux, ipaddr, tport, atoi(bestpopsAux))){
                                                deleteQueryID(queryIdAux);
                                                DadLeft(&root, &fdAccessServer, &fdUp);
                                                continue;

                                            }
                                        }
                                    }
                                    else{
                                         // Run the list of clients to send the message to search for more bestpops left
                                        for(int j = 0; j < tcpsessions; j++){
                                            if(clients.fd[j] != 0)
                                                if(!POP_QUERYclients(clients.fd[j], queryIdAux, atoi(bestpopsAux)))
                                                    removeChild(j);
                                        }
                                        // Insert the pending request bestpops
                                        insertQueryID(queryIdAux, atoi(bestpopsAux) - clients.available);

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

                        else if(bufferUp[0] == 'R' && bufferUp[1] == 'E'){
                            printf("I received a POP_QUERYclients\n");
                            int newLine = 0;
                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0) {
                                // checks if both informations are contained there
                                if(sscanf(&bufferUp[3], "%[^:]:%[^\n]\n", availableIAmRootIP, availableIAmRootPort) == 2) {
                                    printf("availableIAmRootIP: %s -availableIAmRootPort: %s\n", availableIAmRootIP, availableIAmRootPort);
                                    close(fdUp);
                                    fdUp = -1;

                                    int tries = 0;

                                    do{
                                         fdUp = connectToTcp(availableIAmRootIP, availableIAmRootPort);
                                         if(fdUp == -1)
                                            WHOISROOT(&root, &fdAccessServer, &fdUp);
                                        tries++;

                                    }while(fdUp == -1 && tries < TRIES);


                                    
                                    // Clears the idStram string, since it's jobs is done
                                    memset(availableIAmRootIP, '\0', IP_SIZE);
                                    memset(availableIAmRootPort, '\0', BUFFER_SIZE);
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
                            broken = 1;

                            printf("Stream Stop .. Wait a moment! \n");

                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0) {

                                if(status == CONFIRMATION){
                                    close(fdUp);
                                    fdUp = -1;
                                    DadLeft(&root, &fdAccessServer, &fdUp);
                                    continue;
                                }
                               
                               for(int j = 0; j < tcpsessions; j++)
                                    if(clients.fd[j] != 0)
                                        if(!BROKEN_STREAM(clients.fd[j]))
                                            removeChild(j);

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
                        else if(bufferUp[0] == 'S' && bufferUp[1] == 'F'){
                            int newLine = 0;
                            broken = 0;

                            printf("Stream recovered. Enjoy it \n");

                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0) {

                                if(status == CONFIRMATION)
                                    status = NORMAL;

                               
                                for(int j = 0; j < tcpsessions; j++)
                                    if(clients.fd[j] != 0)
                                        if(!STREAM_FLOWING(clients.fd[j]))
                                            removeChild(j);

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
                        else if(bufferUp[0] == 'T' && bufferUp[1] == 'Q'){
                            int newLine = 0;

                            printf("I received a TREE_QUERY\n");

                            // Found a complete message
                            if((newLine = findsNewLine(bufferUp, PACKAGE_TCP)) >= 0) {
                                if(sscanf(&bufferUp[3], "%[^:]:%[^\n]\n", TQip, TQport) == 2) {
                                    printf("TQip: %s TQport: %s\n", TQip, TQport);

                                    // Checks if the TREE_QUERY destination is the own
                                    if((strcmp(ipaddr, TQip) == 0) && (strcmp(tport, TQport) == 0)) {
                                        // Sends a tree reply
                                        if(!TREE_REPLY(fdUp)) {
                                            // CHAMAR WHO IS ROOT
                                        }

                                    }
                                    else {
                                        // Since he is not the destiny, retransmit the message to its childs
                                        for(int j = 0; j < tcpsessions; j++)
                                            if(clients.fd[j] != 0)
                                                if(writeTcp(clients.fd[j], bufferUp, newLine + 1) != newLine + 1)
                                                    removeChild(j);
                                    }
                                    // Clears the idStram string, since it's jobs is done
                                    memset(TQip, '\0', BUFFER_SIZE);
                                    memset(TQport, '\0', BUFFER_SIZE);
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

                                        if(broken)
                                            BROKEN_STREAM(clients.fd[i]);
                                        else
                                            STREAM_FLOWING(clients.fd[i]);

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


                                        printListQId();
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
                                                if(!POP_REPLY(fdUp,queryIdAux,newPopIp, newPopPort, availsSend)){                                            
                                                    deleteQueryID(queryIdAux);
                                                    DadLeft(&root, &fdAccessServer, &fdUp);
                                                    continue;
                                                }
                                                else{
                                                    for(int j = 0; j < availsSend; j++)
                                                        decrementQueryID(queryIdAux);
                                                }
                                            }
                                        }

                                        printListQId();

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
                            // Receives a TREE-REPLY
                            else if(clients.buffer[i][0] == 'T' && clients.buffer[i][1] == 'R'){
                                printf("Received a TREE_REPLY\n");
                                printf("recebi um %s \n", clients.buffer[i]);

                                int doubleNewLine = 0;
                                // Found a complete message
                                if((doubleNewLine = findsDoubleNewLine(clients.buffer[i], PACKAGE_TCP)) >= 0) {
                                    // checks if both informations are contained there
                                    printf("sim entrei \n");
                                    if(root) {
                                        int newLine = findsNewLine(clients.buffer[i], PACKAGE_TCP);
                                        // prints the content of the message
                                        if(sscanf(&clients.buffer[i][3],"%[^:]:%[^ ] %[^\n]\n", TRip, TRport, TRtcpsessions) == 3) {
                                            printf("%s:%s (%s", TRip, TRport, TRtcpsessions);
                                            for((newLine = findsNewLine(clients.buffer[i], PACKAGE_TCP)); newLine <  doubleNewLine - 1; newLine = findsNewLine(clients.buffer[i], PACKAGE_TCP)) {
                                                if(sscanf(&clients.buffer[i][newLine+3],"%[^:]:%[^\n]\n", TRip, TRport) == 2) {
                                                    printf(" %s:%s", TRip, TRport);
                                                    // Sends the message of TREE QUERY to discover the suns of the sun
                                                    TREE_QUERY(clients.fd[i], TRip, TRport);
                                                }
                                                else
                                                    break;
                                            }
                                            printf(")\n");
                                        }    
                                        memset(TRip, '\0', BUFFER_SIZE);
                                        memset(TRport, '\0', BUFFER_SIZE);
                                        memset(TRtcpsessions, '\0', BUFFER_SIZE);
                                    }
                                    else {
                                        // Reenvia a mensagem para cima
                                        if(writeTcp(fdUp, clients.buffer[i], doubleNewLine + 1) == -1) {
                                            // CHAMAR WHO IS ROOT
                                        }
                                    }

                                    // checks if there is another message
                                    if(clients.buffer[i][doubleNewLine + 1] != '\0') {
                                        // Copies the buffer to the beggining
                                        strcpy(clients.buffer[i], &clients.buffer[i][doubleNewLine + 1]);
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

                if(root)
                    printListCLient();

                
            }
            else {
                printf("I received something, but didn't read the message\n");
            }
 
        }

    }

	return 0;
}