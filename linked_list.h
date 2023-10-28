#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include "msg_prints.h"
#include "main_aux.h"

/*
 * This module contains a linked list implementation.
 */

struct linked_list_struct{int data; struct linked_list_struct* next;};
typedef struct linked_list_struct LINKEDNODE;
typedef LINKEDNODE* LINKEDLIST;

/*
 * Returns: a pointer to a LINKEDNODE with the data=data and next = NULL
 * DO NOT FORGET TO FREE RESOURCES USING destoryLinkedList.
 */
LINKEDLIST newLinkedList(int data);

/*
 * destroys all nodes in the list and frees allocated memory.
 */
void destroyLinkedList(LINKEDLIST list);

/*
 * Returns: a pointer to the first LINKEDNODE in the list with data == value.  NULL if non such value was found or the list was empty.
 * clarification: if a = 1->2->3, searchLinkedList(a,2) will yield 2->3.
 */
LINKEDLIST searchLinkedList(LINKEDLIST list, int value);

/*
 * If NULL is given, *list will be treated as an empty list.
 * adds a new node with data = value at the end of the list.
 */
void appendToLinkedList(LINKEDLIST *list,int value);

/*
 * removes the LINKEDLIST toBeRemoved from list.
 * after the function is called, list will not contain toBeRemoved anymore, and toBeRemoved will be a singleton.
 * if toBeRemoved is not in list, nothing will change.
 * toBeRemoved is also destroyed.
 */
void removeFromLinkedList(LINKEDLIST *list, LINKEDLIST toBeRemoved);

/*
 * returns a LINKEDLIST representing the i-th value of the list, similar to searchLinkedList.
 * input of index=0 will return the first value, like the indexing in arrays.
 * if index<0 or grater the length-1, returns NULL. ("outOfBoundsException")
 */
LINKEDLIST getLinkedListByIndex(LINKEDLIST list, int index);

/*
 * Connects list2 to the end of list1.
 * Meaning if 'end' is the last node of list1:
 * before: end->next is NULL.
 * after: end->next is list2.
 */
void linkedListUnion(LINKEDLIST *list1,LINKEDLIST list2);

/*
 * prints the list in the following format: [ val1 val2 val3 ... ]
 */
void printLinkedList(LINKEDLIST list);

/*
 * return the  length of the list. empty list is considered to be of length 0.
 */
int getLength(LINKEDLIST list);

/*
 * Gets a linked list head and a pre-allocated array with the same length as the linked list.
 * The function stores the values of the linked list inside 'target', in order.
 */
void linked_list_to_array(LINKEDLIST list, int* target);

#endif /* LINKED_LIST_H_ */
