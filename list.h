#ifndef LIST_H_INCLUDE
#define LIST_H_INCLUDE

// Library for special types of Int
#include<stdio.h>
#include<stdlib.h>

#define MAX_SIZE 10

typedef struct linked_list
{
    char value[MAX_SIZE];
    int left;
    struct linked_list *next;
} node;

//void create_linked_list(char _value[]);
void print_linked_list();
//void insert_at_last(char  _value[]);
void insert_at_first(char _value[], int left);
void delete_item(char _value[]);
void decrementItem(char _value[]);
int getLeft(char _value[]);

extern node *head;
extern node *last;


#endif