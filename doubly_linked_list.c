#include "doubly_linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include "definitions.h"
#include "msg_prints.h"
#include "main_aux.h"

/*
 * This module contains the doubly linked list and stack implementations.
 */

static doublyLinkedNode* newDoublyLinkedNode(void* data)
{
	/*
	 * returns a pointer to a new node with the data pointer data
	 */
	doublyLinkedNode* pnode = (doublyLinkedNode*)calloc(1,sizeof(doublyLinkedNode));
	check_alloc(pnode,"newDoublyLinkedNode");
	pnode->data = data;
	return pnode;
}

void appendToDoublyLinkedList(doublyLinkedList *list, void* data)
{
	/*
	 * appends a new node with data pointer data to the doubly linked list pointer list.
	 * NULL is considered an empty list.
	 * assumes list points to the last element
	 * Does not change (*list) to point at the new node.
	 */
	doublyLinkedNode* pnode = newDoublyLinkedNode(data);
	if(list == NULL || *list == NULL)
	{
		*list = pnode;
		return;
	}
	(*list)->next = pnode;
	pnode->prev = *list;
}

void push(doublyLinkedList *list, void* data)
{
	/*
	 * Treats the doublyLinkedList as a stack, where *stack points to the head of it.
	 * Adds the new value at the end of the list like appendToDoublyLinkedList, except it also changes the head to that new node.
	 * [original](*stack)->next == *stack
	 * stack == NULL or *stack = NULL are considered empty list and will be treated the same way as appendToDoublyLinkedList.
	 */
	doublyLinkedNode* pnode = newDoublyLinkedNode(data);
	if(list == NULL || *list == NULL)
	{
		*list = pnode;
		return;
	}
	(*list)->next = pnode;
	pnode->prev = *list;
	*list = pnode;
}

void destroyDoublyLinkedList(doublyLinkedList list)
{
	/*
	 * frees up all allocated memory from list and on (frees node, then node->next...)
	 * If this is not the first node, the previous nodes will remain untouched,
	 * except now the next of the last of them points to NULL.
	 */
	if(list!=NULL)
	{
		destroyDoublyLinkedList(list->next);
		if(list->prev != NULL)
		{
			(list->prev)->next = NULL;
		}
		free(list->data);
		free(list);
	}
}

void destroyDoublyLinkedList_backwards(doublyLinkedNode *end)
{
	/*
	 * frees up all allocated memory from "end" and backwards (frees node, then node->prev...).
	 * If this is not the last node, the next nodes will remain untouched except now
	 * the prev of the first of them points to NULL.
	 */
	if(end!=NULL)
	{
		destroyDoublyLinkedList_backwards(end->prev);
		if(end->next != NULL)
		{
			(end->next)->prev = NULL;
		}
		free(end->data);
		free(end);
	}
}

void addAndMakeHead(doublyLinkedList *list, void* data)
{
	/*
	 * clears all nodes following *list and then makes a new node at the new end.
	 * Updates *list (the doubly linked list object) to point at the new node. Meaning (*list) points at the new node.
	 * list == NULL and *list == NULL are considered empty and will be treated the same as in appendToDoublyLinkedList
	 */
	if(*list != NULL)
	{
		destroyDoublyLinkedList((*list)->next); /*clear all following nodes*/
	}
	push(list, data);
}

void* pop(doublyLinkedList *stack)
{
	/*
	 * Treats the doublyLinkedList as a stack.
	 * *stack points to the head (i.e (*stack)->next == NULL) of it.
	 * Removes the head (last item) and returns its value.
	 * Updates the stack's head to point at the new head.
	 * If stack == NULL or *stack==NULL, returns NULL as well.
	 */
	void* data;
	doublyLinkedList updated_stack;

	if(stack == NULL || *stack == NULL)
	{
		return NULL;
	}

	data = (*stack)->data;
	updated_stack = (*stack)->prev;

	if(updated_stack!=NULL)
	{
		updated_stack->next = NULL;
	}
	free(*stack);

	*stack = updated_stack;

	return data;

}
