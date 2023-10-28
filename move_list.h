#ifndef MOVE_LIST_H_
#define MOVE_LIST_H_

#include <stdlib.h>
#include <string.h>
#include "doubly_linked_list.h"
#include "definitions.h"


/*
 * This module is responsible for storing and manipulating the game's move list.
 * The move list is implemented with a doubly linked list.
 */

typedef struct move_t{
	char *command_info; /*Holds the user's command that was executed successfully (for example "generate 6 3").*/
	doublyLinkedNode *set_list_end;/*holds all the sets which were done during the command.
							   *A doubly linked list of ijval structures.
							   *set_list_end itself points to the last set done*/
}move;

/*
 * Data type for the moves list.
 * A set node with (i,j, prev_val, ne_val) represents
 * the change that was made after a call to set i j ne_val:
 * the cell (i,j) had the value prev_val and after
 * the set was executed has the value ne_val .
 */
typedef struct ijval_t{
	int i;/*column number starting from 1*/
	int j;/*row number starting from 1*/
	int prev_val;/*cell value before the command*/
	int ne_val;/*cell value after the command*/
}ijval;

/*
 * A pointer to the head of the moves list.
 * A new move is added whenever "set", "autofill", "generate", or "guess" is executed successfully.
 * A move is added even if the command did not change the state of the board, as long as
 * the command was executed without errors.
 * See recent_move's DOC for more info.
 */
extern doublyLinkedList move_list_head;

/*
 * A doubly linked list that holds the moves done by the user, that changed the board.
 * Each "move node" contains the command as a string and a doubly linked list of all the sets that were done by the command.
 * The move list points to the move that holds the current state of the board.
 * The moves are calls to the functions: set, guess, generate, autofill; that CHANGED the board.
 * For example, if the user entered "guess_hint 5 4" and the board was changed:
 * recent_move->command_info will point to "guess_hint 5 4" (without any additional delimiters like \t\r\n or a space at the end)
 * recent_move->set_list_end will point to the last set that was done during the execution of "guess_hint 5 4".
 * 							 The rest of the sets are in the nodes before set_list_end.
 * If the user set a cell to x and the previous value was x, it doesn't count as a change.
 */
extern doublyLinkedNode *recent_move;

/*
 * This is the "constructor" for an ijval object.
 * Uses malloc. The destroy function is the free function.
 */
ijval* new_ijval(int i, int j, int prev_val, int ne_val);

/*
 * Allocates memory to a new move with the data: (command_info, set_list_end = NULL),
 * destroys all moves after recent_move, appends the new move to the move list (right after recent_move),
 * and updates recent_move to point at the new move.
 * On a memory allocation error prints the error message and terminates the program.
 */
void append_new_move(char* command_info);

/*
 * Revert the action of appending a new move.
 */
void revert_new_move(sudoku_board *board, STATUS s);

/*
 * Allocates memory to a new ijval structure with the data: (i,j,pre_val,ne_val),
 * destroys all moves after set_list_end, appends the new ijval struct
 * to the set list (right after set_list_end),
 * and updates set_list_end to point at the new ijval struct.
 * On a memory allocation error prints the error message and terminates the program.
 */
void append_ijval_to_recent_move(int i, int j, int prev_val, int ne_val);

/*
 * Makes recent_move and move_list_head point to NULL.
 * The previous move list pointers are ignored (not freed, in case they will be needed).
 */
void initialize_move_list();

/*
 * head is the head of the move list you want freed.
 * The method frees all allocated memory inside the list from head and on:
 * For each move - frees set_list, then frees the move structure.
 * If this is not the first node, the previous nodes will remain untouched except now the last of them points to NULL.
 */
void free_move_list(doublyLinkedList head);

/*
 * Assigns all previous values in the move to the board's cells (undoing this move),
 * and updates the errors.
 * Does not change the recent_move pointer.
 * Assumes board is initialized.
 */
void restore_prev_values_of_move(sudoku_board *board, doublyLinkedNode *move, STATUS s);

/*
 * Assigns all next values in the move to the board's cells (redoing this move),
 * and updates the errors.
 * Does not change the recent_move pointer.
 */
void restore_next_values_of_move(sudoku_board *board, doublyLinkedNode *move, STATUS s);

/*
 * A debugging print method.
 * Prints given move node with the format:
 *"Command: command_info
 * set i j prev_val next_val
 * set i j prev_val next_val
 * ...
 *"
 */
void print_move_node(doublyLinkedNode *move_node);

/*
 * A debugging print method.
 * Prints the set list with the format:
 *"set i j prev_val next_val
 * set i j prev_val next_val
 * ...
 *"
 * From the last set to the first.
 */
void print_set_list_backwards(doublyLinkedList end);

#endif /* MOVE_LIST_H_ */
