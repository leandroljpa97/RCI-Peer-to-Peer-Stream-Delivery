
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


node *head=NULL, *last=NULL;


/*
    User Defined Functions
*/

/*void create_linked_list(char val[]){

    insert_at_last(val);

}


void insert_at_last(char  _value[]){
    node *temp_node;
    temp_node = (node *) malloc(sizeof(node));

    strcpy(temp_node->value,_value);
    temp_node->next=NULL;

    //For the 1st element
    if(head==NULL)
    {
        head=temp_node;
        last=temp_node;
    }
    else
    {
        last->next=temp_node;
        last=temp_node;
    }

}

*/

void insert_at_first(char  _value[], int _left){
    node *temp_node = (node *) malloc(sizeof(node));

    strcpy(temp_node->value, _value);
    temp_node->next = head;
    temp_node ->left = _left;

    head = temp_node;
}



void delete_item(char  _value[]){
    node *myNode = head, *previous=NULL;
    int flag = 0;

    while(myNode!=NULL)
    {
        if(!strcmp(myNode->value,_value))
        {
            if(previous==NULL)
                head = myNode->next;
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


void decrementItem(char _value[]){
    node *searchNode = head;

    while(searchNode!=NULL)
    {
        if(!strcmp(searchNode->value, _value))
        {
            searchNode->left = searchNode->left -1;
            if(searchNode->left == 0)
                delete_item(_value);
                return;
        }
        else
            searchNode = searchNode->next;
    }

    

}

int getLeft(char _value[]){
    node *searchNode = head;

    while(searchNode!=NULL)
    {
        if(!strcmp(searchNode->value, _value))
            return searchNode->left;
        else
            searchNode = searchNode->next;
    }
    return 0;

}


void print_linked_list()
{
    printf("\nYour full linked list is\n");

    node *myList;
    myList = head;

    while(myList!=NULL)
    {
        printf("%s ", myList->value);

        myList = myList->next;
    }
    puts("");
}