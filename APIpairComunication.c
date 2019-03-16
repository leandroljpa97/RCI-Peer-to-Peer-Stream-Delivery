#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "APIpairComunication.h"
#include "utils.h"
#include "tcp.h"

#define HEX_STYLE "0000"

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

    if(writeTcp(_fd, buffer, i + 1) != i + 1) 
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
    if(writeTcp(_fd, buffer, i + 1) != i + 1) 
        return 0;

    printf("sent a NEW_POP\n");

    return 1;
}

int REDIRECT(int _fd, char _ipaddr[], char _tport[]) {
    char buffer[BUFFER_SIZE];

    // Creates REDIRECT message
    strcpy(buffer, "RD ");
    strcat(buffer, _ipaddr);
    strcat(buffer,":");
    strcat(buffer, _tport);
    strcat(buffer,"\n");

    // finds the size of the NEW_POP message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if(writeTcp(_fd, buffer, i + 1) != i + 1) 
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

    if(writeTcp(_fd, buffer, i + 1) != i + 1) 
        return 0;

    printf("sent a REDIRECT\n");

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

    if(writeTcp(_fd, buffer, i) != i) 
        return 0;

    printf("sent DATA\n");

    return 1;
}


/* DISCORVERY OF THE ACCESS POINT */

int POP_QUERY(int _fd, uint16_t _queryID) {
	char buffer[PACKAGETCP];
	char tcpsessionsString[] = "00";
	char queryIDHex[] = "0000";

    // Converts queryID to hex format
    convertNumDoHex(queryIDHex, _queryID);
	printf("%d in Hexadecimal = %s", _queryID, queryIDHex);

	// Creates POP_QUERY message
    strcpy(buffer, "PQ ");
    strcat(buffer, queryIDHex);
    strcat(buffer, " ");
    sprintf(tcpsessionsString, "%d", tcpsessions);
    strcat(buffer, tcpsessionsString);
    strcat(buffer, "\n");
    printf("o buffer no whoIsRoot é %s\n", buffer);

    // finds the size of the POP_QUERY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if(i + 1 != writeTcp(_fd, buffer, i + 1)) {
    	printf("Fail sending POP_QUERY to fd %d\n", _fd);
    	return 0;
	}

    return 1;
}

int POP_REPLY(int _fd, uint16_t _queryID, int avails) {
	char buffer[PACKAGETCP];
	char availsString[] = "00";

	// Converts queryID to hex format
	char queryIDHex[] = "HEX_STYLE";
	sprintf(queryIDHex, "%hu",  (unsigned int) _queryID);
	printf("%d in Hexadecimal= %s", _queryID, queryIDHex);

	// Creates POP_REPLY message
    strcpy(buffer, "PR ");
    strcat(buffer, queryIDHex);
    strcat(buffer, " ");
    strcat(buffer, ipaddr);
    strcat(buffer, ":");
    strcat(buffer, tport);
    strcat(buffer, " ");
    sprintf(availsString, "%d", avails);
    strcat(buffer, availsString);
    strcat(buffer, "\n");
    printf("o buffer no whoIsRoot é %s\n", buffer);

    // finds the size of the POP_REPLY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if(i + 1 != writeTcp(_fd, buffer, i + 1)) {
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

    if(i + 1 != writeTcp(_fd, buffer, i + 1)) {
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
    printf("o buffer no TREE_QUERY é %s\n", buffer);

    // finds the size of the POP_REPLY message
    int i = 0;
    for(i = 0; buffer[i] != '\0'; ++i);

    if(i + 1 != writeTcp(_fd, buffer, i + 1)) {
        printf("Fail sending POP_REPLY\n");
        return 0;
    }

    return 1;
}