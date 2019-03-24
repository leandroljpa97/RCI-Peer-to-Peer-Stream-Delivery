#ifndef LIST_H_INCLUDE
#define LIST_H_INCLUDE

// Library for special types of Int
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define MAX_SIZE 10


typedef struct _queryIDList {
    char queryID[MAX_SIZE];
    int left;
    struct _queryIDList *next;
} queryIDList_t;

typedef struct _clientList {
    char ip[IP_SIZE];
    char port[PORT_SIZE];
    int bestpops;
    // Indicates how many times this AP was given after there's only bestpops items on the list
    int negative;
    struct _clientList *next;
} clientList_t;

extern clientList_t *accessPoints;
extern queryIDList_t *queryIDList;


void insertQueryID(char _queryID[], int _left);

void decrementQueryID(char _queryID[]);

int getLeftQueryID(char _queryID[]);

void deleteQueryID(char  _queryID[]);

int isAPontTheList(char _port[], char _ip[], int _bestpops);

void insertAccessPoint(char _ip[], char  _port[], int _bestpops) ;

void deleteClientAP(clientList_t  *removeIP);

int getAccessPoint(char *ip, char *port);

void removeNode(char * ip, char *port);


#endif