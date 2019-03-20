#ifndef LIST_H_INCLUDE
#define LIST_H_INCLUDE

// Library for special types of Int
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define MAX_SIZE 10

#define ERASELIMIT 1

typedef struct _queryIDList {
    char queryID[MAX_SIZE];
    int left;
    struct _queryIDList *next;
} queryIDList_t;

typedef struct _clientList {
    char ip[IP_SIZE];
    char port[PORT_SIZE];
    int bestpops;
    struct _clientList *next;
} clientList_t;

extern clientList_t *accessPoints;
extern queryIDList_t *queryIDList;


void insertQueryID(char _queryID[], int _left);

void decrementQueryID(char _queryID[]);

int getLeftQueryID(char _queryID[]);

void deleteQueryID(char  _queryID[]);

void insertAccessPoint(char _port[], char _ip[], int _bestpops);

int getAccessPoint(char *ip, char *port);

#endif