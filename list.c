
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "list.h"


clientList_t *accessPoints;
queryIDList_t *queryIDList;


void insertQueryID(char _queryID[], int _left) {
    queryIDList_t *temp_node = (queryIDList_t *) malloc(sizeof(queryIDList_t));
    if(temp_node == NULL) {
        printf("Error malloc\n");
        exit(0);
    }

    strcpy(temp_node->queryID, _queryID);
    temp_node->left = _left;

    temp_node->next = queryIDList;
    queryIDList = temp_node;
}

void decrementQueryID(char _queryID[]) {
    queryIDList_t *searchNode = queryIDList;
    
    // Searchs for the QueryID
    while(searchNode != NULL)
    {
        if(!strcmp(searchNode->queryID, _queryID))
        {
            searchNode->left = searchNode->left -1;
            if(searchNode->left == 0) {
                deleteQueryID(_queryID);
                return;
            }
        }
        else
            searchNode = searchNode->next;
    }
}


void deleteQueryID(char  _queryID[]) {
    queryIDList_t *myNode = queryIDList, *previous=NULL;
    int flag = 0;

    while(myNode!=NULL)
    {
        if(!strcmp(myNode->queryID,_queryID))
        {
            if(previous==NULL)
                queryIDList = myNode->next;
            else
                previous->next = myNode->next;


            flag = 1;
            free(myNode); //need to free up the memory to prevent memory leak
            break;
        }

        previous = myNode;
        myNode = myNode->next;
    }

    if(flag==0)
        printf("Key not found!\n");
}

int getLeftQueryID(char _queryID[]) {
    queryIDList_t *aux = queryIDList;

    while(aux != NULL) {
        if(!strcmp(aux->queryID, _queryID)) {
            return aux->left;
        }
        aux = aux->next;
    }
    return 0;
}



void insertAccessPoint(char _port[], char _ip[], int _bestpops) {
    clientList_t *aux = accessPoints;

     while(aux!=NULL)
    {
        if((strcmp(aux->port,_port) == 0) && (strcmp(aux->ip,_ip) == 0)) {
            aux->bestpops = _bestpops;
            return;
        }
        aux = aux->next;
    }

    aux = accessPoints;

    clientList_t *new = (clientList_t *) malloc(sizeof(clientList_t));
    if(new == NULL) {
        printf("Error malloc\n");
        exit(0);
    }
    new->next = accessPoints;
    
    strcpy(new->ip, _ip);
    strcpy(new->port, _port);
    new->bestpops = _bestpops;

    accessPoints = new;
}

/* 
    returns: 1 when it needs to find more things to populate the list
             0 success
             -1 doesnt have an IP
*/
int getAccessPoint(char *ip, char *port) {
    clientList_t *aux1 = accessPoints;
    clientList_t *aux2 = accessPoints;
    int numberOfAP = 0;

    if(aux1 != NULL) {
        numberOfAP = 1;
        // Finds the last element of the list
        while(aux1->next != NULL) {
            aux2 = aux1;
            aux1 = aux1->next;
            numberOfAP++;
        }
    }
    else
        return -1;

    // Sends the information from the last element on the list
    strcpy(ip, aux1->ip);
    strcpy(port, aux1->port);

    // Decreases the number of available connections
    if(aux1->bestpops > 1) {
        aux1->bestpops--;
    }
    else {
        // There's more than one element on the list -> delete element
        if(aux1 != aux2) {
            aux2->next = NULL;
            free(aux1);
        }
    }

    if(numberOfAP <= ERASELIMIT) 
        return 1;
    
    return 0;
}

