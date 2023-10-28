#ifndef DOUBLY_LINKED_LIST_H_
#define DOUBLY_LINKED_LIST_H_

/*
 * This module contains the doubly linked list and stack implementations.
 */


/*next <-node(data)-> prev*/
struct doublyLinkedNode_t{struct doublyLinkedNode_t* prev;struct doublyLinkedNode_t* next; void* data;};
typedef struct doublyLinkedNode_t doublyLinkedNode;
typedef doublyLinkedNode* doublyLinkedList;

/*
 * appends a new node with data pointer data to the doubly linked list pointer list.
 * NULL is considered an empty list.
 * assumes list points to the last element
 * Does not change (*list) to point at the new node.
 */
void appendToDoublyLinkedList(doublyLinkedList *list, void* data);

/*
 * frees up all allocated memory from list and on (frees node, then node->next...)
 * If this is not the first node, the previous nodes will remain untouched,
 * except now the next of the last of them points to NULL.
 */
void destroyDoublyLinkedList(doublyLinkedList list);

/*
 * frees up all allocated memory from "end" and backwards (frees node, then node->prev...).
 * If this is not the last node, the next nodes will remain untouched except now
 * the prev of the first of them points to NULL.
 */
void destroyDoublyLinkedList_backwards(doublyLinkedNode *end);

/*
 * clears all nodes following *list and then makes a new node at the new end.
 * Updates *list (the doubly linked list object) to point at the new node. Meaning (*list) points at the new node.
 * list == NULL and *list == NULL are considered empty and will be treated the same as in appendToDoublyLinkedList
 */
void addAndMakeHead(doublyLinkedList *list, void* data);

/*
 * Treats the doublyLinkedList as a stack.
 * *stack points to the head (i.e (*stack)->next == NULL) of it.
 * Removes the head (last item) and returns its value.
 * Updates the stack's head to point at the new head.
 * If stack == NULL or *stack==NULL, returns NULL as well.
 */
void* pop(doublyLinkedList *stack);

/*
 * Treats the doublyLinkedList as a stack, where *stack points to the head of it.
 * Adds the new value at the end of the list like appendToDoublyLinkedList, except it also changes the head to that new node.
 * [original](*stack)->next == *stack
 * stack == NULL or *stack = NULL are considered empty list and will be treated the same way as appendToDoublyLinkedList.
 */
void push(doublyLinkedList *stack, void* data);

#endif /* DOUBLY_LINKED_LIST_H_ */
