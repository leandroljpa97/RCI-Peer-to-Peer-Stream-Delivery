#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <arpa/inet.h>


#include "APIpairComunication.h"
#include "utils.h"
#include "tcp.h"
#include "list.h"

#define HEX_STYLE "0000"

void removeChild(int index){
    removeNode(clients.ip[index], clients.port[index]);
    close(clients.fd[index]);
    clients.fd[index] = 0;
    clients.available ++;

    memset(clients.ip[index], '\0', IP_SIZE);
    memset(clients.port[index],'\0', PORT_SIZE);
    memset(clients.buffer[index], '\0', PACKAGETCP);
}

void closeAllClients() {
    for (int i = 0; i < tcpsessions; ++i) {
        if(clients.fd[i] != 0)
            removeChild(i);
    }
}



/* ENTERING STREAM TREE */

int WELCOME(int _fd) {
    char buffer[BUFFER_SIZE];

    // Creates WELCOME message
    strcpy(buffer,"WE ");
    strcat(buffer,streamId);
    strcat(buffer,"\n");

    // finds the size of the WHOISROOT message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) 
        return 0;

    printf("recebi um novo cliente  \n");

    return 1;
}

int NEW_POP(int _fd) {
    char buffer[BUFFER_SIZE];

    // Creates NEW_POP message
    strcpy(buffer, "NP ");
    strcat(buffer, ipaddr);
    strcat(buffer,":");
    strcat(buffer, tport);
    strcat(buffer,"\n");

    // finds the size of the NEW_POP message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    printf("o buffer é %s \n", buffer);
    if((writeTcp(_fd, buffer, i + 1)) == -1) 
        return 0;

    printf("sent a NEW_POP\n");

    return 1;
}

int REDIRECT(int _fd, char _ipaddr[], char _tport[]) {
    char buffer[BUFFER_SIZE];

    // Creates REDIRECT message
    strcpy(buffer, "RE ");
    strcat(buffer, _ipaddr);
    strcat(buffer,":");
    strcat(buffer, _tport);
    strcat(buffer,"\n");

    // finds the size of the NEW_POP message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1))== -1) 
        return 0;

    printf("sent a REDIRECT\n");

    return 1;
}

/* STOP and RESTABLESHIMENT of STREAMING */

int STREAM_FLOWING(int _fd) {
    char buffer[BUFFER_SIZE];

    // Creates STREAM_FLOWING message
    strcpy(buffer, "SF");
    strcat(buffer,"\n");

    // finds the size of the NEW_POP message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) 
        return 0;

    printf("sent a Stream Flowing\n");

    return 1;
}

int BROKEN_STREAM(int _fd){
    char buffer[BUFFER_SIZE];

    // Creates BROKEN_STREAM message
    strcpy(buffer, "BS");
    strcat(buffer,"\n");

    // finds the size of the BROKEN_STREAM message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) 
        return 0;

    printf("sent a broken Stream\n");

    return 1;

}

int DATA(int _fd, int nbytes, char _data[]) {
    char buffer[PACKAGETCP];
    char nbytesHex[] = "0000";

    convertNumDoHex(nbytesHex, nbytes);

    // Creates STREAM_FLOWING message
    strcpy(buffer, "DA ");
    strcat(buffer, nbytesHex);
    strcat(buffer,"\n");
    strcat(buffer, _data);

    printf("dentro da func DATA: %d\n", (int) strlen(_data));
   
    // finds the size of the NEW_POP message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    printf("i %d, a data que vou enviar é : %s \n", i ,buffer);

    if((writeTcp(_fd, buffer, i)) == -1) 
        return 0;

    printf("sent DATA\n");

    return 1;
}


/* DISCORVERY OF THE ACCESS POINT */

int POP_QUERYroot(int _fd, uint16_t _queryId, int _bestPops) {
	char buffer[PACKAGETCP];
	char bestPopString[] = "00";
    char queryIDHex[] = "0000";

    convertNumDoHex(queryIDHex, _queryId);

	// Creates POP_QUERY message
    strcpy(buffer, "PQ ");
    strcat(buffer, queryIDHex);
    strcat(buffer, " ");
    sprintf(bestPopString, "%d", _bestPops);
    strcat(buffer, bestPopString);
    strcat(buffer, "\n");
    printf("o buffer no whoIsRoot é %s\n", buffer);

    // finds the size of the POP_QUERY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) ==-1) {
    	printf("Fail sending POP_QUERY to fd %d\n", _fd);
    	return 0;
	}

    return 1;
}

int POP_QUERYclients(int _fd, char _queryId[], int _bestPops) {
    char buffer[PACKAGETCP];
    char bestPopString[] = "00";

    // Creates POP_QUERY message
    strcpy(buffer, "PQ ");
    strcat(buffer, _queryId);
    strcat(buffer, " ");
    sprintf(bestPopString, "%d", _bestPops);
    strcat(buffer, bestPopString);
    strcat(buffer, "\n");
    printf("o buffer no whoIsRoot é %s\n", buffer);

    // finds the size of the POP_QUERY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) {
        printf("Fail sending POP_QUERY to fd %d\n", _fd);
        return 0;
    }

    return 1;
}

int POP_REPLY(int _fd, char _queryID[], char _ipaddr[], char _tport[], int _avails) {
	char buffer[PACKAGETCP];
	char availsString[] = "00";

	// Creates POP_REPLY message
    strcpy(buffer, "PR ");
    strcat(buffer, _queryID);
    strcat(buffer, " ");
    strcat(buffer, _ipaddr);
    strcat(buffer, ":");
    strcat(buffer, _tport);
    strcat(buffer, " ");
    sprintf(availsString, "%d", _avails);
    strcat(buffer, availsString);
    strcat(buffer, "\n");
    printf("o buffer no whoIsRoot é %s\n", buffer);

    // finds the size of the POP_REPLY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) {
    	printf("Fail sending POP_REPLY\n");
    	return 0;
	}

    return 1;
}

/* TREE STRUCTURE MONITORING */

int TREE_QUERY(int _fd, char _ipaddr[], char _tport[]) {
    char buffer[PACKAGETCP];

    // Creates TREE_QUERY message
    strcpy(buffer, "TQ ");
    strcat(buffer, _ipaddr);
    strcat(buffer, ":");
    strcat(buffer, _tport);
    strcat(buffer, "\n");
    printf("o buffer no TREE_QUERY é %s\n", buffer);

    // finds the size of the POP_REPLY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) {
        printf("Fail sending POP_REPLY\n");
        return 0;
    }

    return 1;
}

int TREE_REPLY(int _fd) {
    char buffer[PACKAGETCP];
    char tcpsessionsString[] = "00";

    // Creates TREE_REPLY message
    strcpy(buffer, "TR ");
    strcat(buffer, ipaddr);
    strcat(buffer, ":");
    strcat(buffer, tport);
    strcat(buffer, " ");
    sprintf(tcpsessionsString, "%d", tcpsessions);
    strcat(buffer, tcpsessionsString);
    strcat(buffer, "\n");
    for (int i = 0; i < tcpsessions; ++i) {
        if(clients.fd[i] != 0) {
            strcat(buffer, clients.ip[i]);
            strcat(buffer, ":");
            strcat(buffer, clients.port[i]);
            strcat(buffer, "\n");
        }
    }
    strcat(buffer, "\n");
    printf("o buffer no TREE_QUERY é %s\n", buffer);

    // finds the size of the POP_REPLY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if((writeTcp(_fd, buffer, i + 1)) == -1) {
        printf("Fail sending POP_REPLY\n");
        return 0;
    }

    return 1;
}

