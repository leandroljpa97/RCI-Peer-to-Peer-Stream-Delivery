
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

// Lists
clientList_t *accessPoints;
queryIDList_t *queryIDList;

// Pointer to client that is going to be given
clientList_t *currentClientAP;


void insertQueryIDroot(int _queryID, int _left) {
    queryIDList_t *temp_node = (queryIDList_t *) malloc(sizeof(queryIDList_t));
    if(temp_node == NULL) {
        printf("Error malloc\n");
        exit(0);
    }

    char queryIDHex[] = "0000";

    convertNumDoHex(queryIDHex, _queryID);

    strcpy(temp_node->queryID, queryIDHex);
    temp_node->left = _left;

    temp_node->next = queryIDList;
    queryIDList = temp_node;
}


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
            searchNode->left -= 1;
            if(searchNode->left == 0)
                deleteQueryID(_queryID);
            return;
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

void deleteQueryIDList() {
    queryIDList_t *aux = queryIDList;
    queryIDList_t *aux2;

    while(aux != NULL) {
        aux2 = aux;
        aux = aux->next;
        free(aux2);
    }
}


/********************* ACCESS POINT ***********************/

/* 
    Funtion checks weather an AP is on the list
    returns: 1 when it's not on the list or the bestpops number is < than the bestpops received to that AP
             0 it's already on the list
*/
int isAPontTheList(char _port[], char _ip[], int _bestpops) {
    clientList_t *aux = accessPoints;

    while(aux!=NULL)
    {
        if((strcmp(aux->port,_port) == 0) && (strcmp(aux->ip,_ip) == 0)) {
            if(_bestpops > aux->bestpops)
                return 1;
            else
                return 0;
        }
        aux = aux->next;
    }

    return 1;
}

void insertAccessPoint(char _ip[], char  _port[], int _bestpops) {
    clientList_t *aux = accessPoints;
    int newAP = 0;
    int plus = 0;

    // Se ja estiver na lista, atualiza o seu valor
    while(aux!=NULL)
    {
        if((strcmp(aux->port,_port) == 0) && (strcmp(aux->ip,_ip) == 0)) {
            plus = _bestpops - aux->bestpops;
            aux->bestpops = _bestpops;
            newAP = 1;
            numberOfAP += plus;
            break;
        }
        aux = aux->next;
    }
    if(newAP == 0) {
        // Creates a new client structre
        clientList_t *new = (clientList_t *) malloc(sizeof(clientList_t));
        if(new == NULL) {
            printf("Error malloc\n");
            exit(0);
        }

        // Populate the structure
        new->next = accessPoints;
        strcpy(new->ip, _ip);
        strcpy(new->port, _port);
        new->bestpops = _bestpops;
        new->negative = 0;
        accessPoints = new;

        if(numberOfAP == 0) {
            currentClientAP = new;
        }
        // Increases the number of AP on the list
        numberOfAP += _bestpops;
    }

    aux = accessPoints;

    // Runs the list in search of negative AP if numberOfAP > bestpops
    if(numberOfAP > bestpops) {
        while(aux!=NULL) {
            // if there's a negative AP
            if(aux->negative > 0) {
                aux->bestpops -= aux->negative;    
                aux->negative = 0;
                if(aux->bestpops <= 0) {
                    if(currentClientAP == aux) {
                         // Goes to the next client on the list
                        if(currentClientAP->next != NULL)
                            currentClientAP = currentClientAP->next;
                        else
                            currentClientAP = accessPoints;
                    }
                    // Deletes the client and reduces the number of AP's
                    deleteClientAP(aux);
                }
                return;
            }
            aux = aux->next;
        }
    }


}

void deleteClientAP(clientList_t  *removeIP) {
    clientList_t *myNode = accessPoints, *previous = NULL;
    int flag = 0;

    while(myNode != NULL) {
        if(myNode == removeIP)
        {
            if(previous == NULL)
                accessPoints = myNode->next;
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

/* 
    Funtion returns the information contained in the currentAP and advances the pointer to the next AP on the list
    returns: 1 when it needs to find more things to populate the list
             0 success
*/
int getAccessPoint(char *ip, char *port) {
    // Sends the information from the currentClientAP

    if(currentClientAP != NULL){
        strcpy(ip, currentClientAP->ip);
        strcpy(port, currentClientAP->port);

        clientList_t *aux = currentClientAP;
        // Goes to the next client on the list
        
        if(currentClientAP->next != NULL)
            currentClientAP = currentClientAP->next;
        else
            currentClientAP = accessPoints;

        // When there's more AP than bestpops, decreses the number of bestpops of that AP or remove it from the list if there's no more positions
        if(numberOfAP > bestpops) {
            // Decreases the number of available connections for that AP
            if(aux->bestpops > 0) {
                aux->bestpops--;
            }
            else {
                deleteClientAP(aux);
            }
        }
        // When there's less or equal AP than bestpops, increases the negative variable
        else {
            aux->negative++;
        }
        numberOfAP--;

        // When there's only bestpops AP on the list, sends the information to do POP_QUERY
        if(numberOfAP < bestpops)
            return 1;

        return 0;

    }
    else
        return 1;
}

void removeNode(char * ip, char *port){
    clientList_t *myNode = accessPoints, *previous=NULL;
    int flag = 0;

    while(myNode!=NULL) {
        if((strcmp(myNode->ip , ip)== 0) && (strcmp(myNode->port , port)==0))
        {
            if(previous==NULL)
                accessPoints = myNode->next;
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

void deleteClient(int _fd) {
    int i;
    for (i = 0; i < tcpsessions; ++i) {
        if(clients.fd[i] == _fd) {
            break;
        }
    }

    if(root)
        removeNode(clients.ip[i], clients.port[i]);

    close(clients.fd[i]);
    clients.fd[i] = 0;
    clients.available ++;

    memset(clients.ip[i], '\0', IP_SIZE);
    memset(clients.port[i],'\0', PORT_SIZE);
    memset(clients.buffer[i], '\0', PACKAGE_TCP);
}

void closeAllClients() {
    for (int i = 0; i < tcpsessions; ++i) {
        if(clients.fd[i] != 0)
            deleteClient(clients.fd[i]);
    }
}

void printListCLient(){

    printf("BEGINNING CLient LIST \n");
    clientList_t *myNode = accessPoints;

    while(myNode != NULL){
        printf("    myNode ->ip: %s, myNode ->port: %s, bestpops:%d \n",myNode->ip, myNode->port, myNode->bestpops);
        myNode = myNode ->next;
    }
        printf("END CLient LIST \n");

}

void printListQId() {
   queryIDList_t *aux = queryIDList;
   printf("QUERY ID LIST\n");
    while(aux != NULL){
        printf("        queryId: %s, left %d \n",aux->queryID, aux->left);
        aux = aux ->next;
    }
    printf("QUERY ID LIST END\n");
}

void deleteAPList() {
    clientList_t  *aux = accessPoints;
    clientList_t *aux2;

    while(aux != NULL) {
        aux2 = aux;
        aux = aux->next;
        free(aux2);
    }
}


