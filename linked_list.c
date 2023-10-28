#include "linked_list.h"

/*
 * This module contains a linked list implementation.
 */

LINKEDLIST newLinkedList(int data)
{
	/*
	 * Returns: a pointer to a LINKEDNODE with the data=data and next = NULL
	 * DO NOT FORGET TO FREE RESOURCES USING destoryLinkedList.
	 */
	LINKEDLIST list;
	list = malloc(sizeof(LINKEDNODE));
	check_alloc(list,"newLinkedList");

	list->data = data;
	list->next = NULL;
	return list;
}

void destroyLinkedList(LINKEDLIST list)
{
	/*
	 * destroys all nodes in the list and frees allocated memory.
	 */
	if(list!=NULL)
	{
		destroyLinkedList(list->next);
		free(list);
	}
}

LINKEDLIST searchLinkedList(LINKEDLIST list, int value)
{
	/*
	 * Returns: a pointer to the first LINKEDNODE in the list with data == value.  NULL if non such value was found or the list was empty.
	 * clarification: if a = 1->2->3, searchLinkedList(a,2) will yield 2->3.
	 */
	while(list!=NULL)
	{
		if(list->data==value)
		{
			return list;
		}
		list = list->next;
	}
	return NULL;
}

void appendToLinkedList(LINKEDLIST *list,int value)
{
	/*
	 * If NULL is given, *list will be treated as an empty list.
	 * adds a new node with data = value at the end of the list.
	 */
	if(list == NULL || *list == NULL)
	{
		*list = newLinkedList(value);
	}
	else
	{
		while((*list)->next!=NULL) /*getting to the end*/
		{
			list = &((*list)->next);
		}
		(*list)->next = newLinkedList(value);
	}
}

void removeFromLinkedList(LINKEDLIST *list, LINKEDLIST toBeRemoved)
{
	/*
	 * removes the LINKEDLIST toBeRemoved from list.
	 * after the function is called, list will not contain toBeRemoved anymore, and toBeRemoved will be a singleton.
	 * if toBeRemoved is not in list, nothing will change.
	 * toBeRemoved is also destroyed.
	 */
	LINKEDLIST pos;
	if(list==NULL ||*list==NULL) {return;}
	if((*list)==toBeRemoved)
	{
		*list = (*list)->next;
	}
	else
	{
		pos = *list;
		while(pos->next!=toBeRemoved)
		{
			if(pos->next==NULL)
			{
				return;
			}
			pos = pos->next;
		}
		pos->next = toBeRemoved->next; /*connects toBeremoved.prev with its next*/

	}
	toBeRemoved->next = NULL; /*disconnects toBeRemoved from the nodes succeeding it*/
	destroyLinkedList(toBeRemoved);

}

LINKEDLIST getLinkedListByIndex(LINKEDLIST list, int index)
{
	/*
	 * returns a LINKEDLIST representing the i-th value of the list, similar to searchLinkedList.
	 * input of index=0 will return the first value, like the indexing in arrays.
	 * if index<0 or grater the length-1, returns NULL. ("outOfBoundsException")
	 */
	int i = 0;
	if(index<0 || list ==NULL)
	{
		return NULL; /*edge cases*/
	}
	for(i = 0;i<index;i++) /*moves index-1 steps forward from the head of the list*/
	{
		list = list->next;
		if(list==NULL)
		{
			return NULL; /*if reached the end of the list it means index is out of bounds*/
		}
	}
	return list;
}

void printLinkedList(LINKEDLIST list)
{
	/*
	 * prints the list in the following format: [ val1 val2 val3 ... ]
	 */
	printf("[ ");
	while(list!=NULL)
	{
		printf("%d ",list->data);
		list = list->next;
	}
	printf("]\n");
}

void linkedListUnion(LINKEDLIST *list1,LINKEDLIST list2)
{
	/*
	 * Connects list2 to the end of list1.
	 * Meaning if 'end' is the last node of list1:
	 * before: end->next is NULL.
	 * after: end->next is list2.
	 */
	LINKEDLIST pos;
	if(list2==NULL)
	{
		return; /*no need to do anything*/
	}
	if(list1==NULL || *list1==NULL)
	{
		*list1 = list2; /*list1 is empty, so now list1 is list2*/
	}
	else
	{
		pos = *list1;
		while(pos->next!=NULL)
		{
			pos = pos->next;
		}
		/*now pos is the last item in list1*/
		pos->next = list2;
	}
}

int getLength(LINKEDLIST list)
{
	/*
	 * return the  length of the list. empty list is considered to be of length 0.
	 */
	int n = 0;
	while(list!=NULL)
	{
		list = list->next;
		n++;
	}
	return n;
}

void linked_list_to_array(LINKEDLIST list, int* target)
{
	/*
	 * Gets a linked list head and a pre-allocated array with the same length as the linked list.
	 * The function stores the values of the linked list inside 'target', in order.
	 */
	LINKEDLIST pos;
	int i = 0;
	for(pos = list; pos!=NULL; pos = pos->next)
	{
		target[i] = pos->data;
		i++;
	}
}
