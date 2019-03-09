#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "APIpairComunication.h"
#include "utils.h"
#include "tcp.h"

#define HEX_STYLE "0000"

int WELCOME(int _fd) {
    char buffer[BUFFER_SIZE];

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

/* DISCORVERY OF THE ACCESS POINT */

int POP_QUERY(int _fd, uint16_t *_queryID) {
	char buffer[PACKAGETCP];
	char tcpsessionsString[] = "00";

	// Converts queryID to hex format
	char queryIDHex[] = "HEX_STYLE";
	sprintf(queryIDHex, "%hu", (unsigned int) *_queryID);
	printf("%d in Hexadecimal= %s", *_queryID, queryIDHex);

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

    *_queryID++;

    return 1;
}

int POP_REPLY(int _fd, uint16_t *_queryID, int avails) {
	char buffer[PACKAGETCP];
	char availsString[] = "00";

	// Converts queryID to hex format
	char queryIDHex[] = "HEX_STYLE";
	sprintf(queryIDHex, "%hu",  (unsigned int) *_queryID);
	printf("%d in Hexadecimal= %s", *_queryID, queryIDHex);

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