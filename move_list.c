#include "move_list.h"
#include "game.h"
#include "msg_prints.h"

/*
 * This module is responsible for storing and manipulating the game's move list.
 * The move list is implemented with a doubly linked list.
 */

/*
 * A pointer to the head of the moves list.
 * See recent_move's DOC for more info.
 */
doublyLinkedList move_list_head = NULL;

/*
 * A doubly linked list that holds the moves done by the user, that changed the board.
 * Each "move node" contains the command as a string and a doubly linked list of all the sets that were done by the command.
 * The move list points to the move that holds the current state of the board.
 * The moves are calls to the functions: set, guess, generate, autofill; that CHANGED the board.
 * For example, if the user entered "guess_hint 5 4" and the board was changed:
 * recent_move->command_info will point to "guess_hint 5 4" (without any additional delimiters like \t\r\n or a space at the end)
 * recent_move->set_list_end will point to the last set that was done during the execution of "guess_hint 5 4".
 * The rest of the sets are in the nodes before set_list_end.
 * If the user set a cell to x and the previous value was x, it doesn't count as a change.
 */
doublyLinkedNode *recent_move = NULL;

ijval* new_ijval(int i, int j, int prev_val, int ne_val)
{
	/*
	 * This is the "constructor" for an ijval object.
	 * Uses malloc. The destroy function is the free function.
	 */
	ijval *new = (ijval*)malloc(sizeof(ijval));
	check_alloc(new,"new_ijval");

	new->i = i;
	new->j = j;
	new->prev_val = prev_val;
	new->ne_val = ne_val;

	return new;
}

static void print_ijval(ijval *ijval)
{
	/*
	 * A debugging print method.
	 * Prints the ijval struct with the format:
	 *"set i j prev_val next_val\n"
	 * Assumes the input is not NULL.
	 */
	printf("set %d %d %d %d\n",ijval->i,ijval->j,ijval->prev_val,ijval->ne_val);
}

void print_set_list_backwards(doublyLinkedList end)
{
	/*
	 * A debugging print method.
	 * Prints the set list with the format:
	 *"set i j prev_val next_val
	 * set i j prev_val next_val
	 * ...
	 *"
	 * From the last set to the first.
	 */
	doublyLinkedNode *node;
	ijval *ijvalp;
	if(end==NULL)
	{
		printf("The end node of set list points to NULL!\n");
		return;
	}
	node = end;
	while(node!=NULL)
	{
		ijvalp = (ijval*)(node->data);
		if(ijvalp==NULL)
		{
			printf("The data pointer of the set node points to NULL!\n");
			return;
		}
		print_ijval(ijvalp);
		node = node->prev;
	}
	printf("Reached end of set list.\n");
}

void print_move_node(doublyLinkedNode *move_node)
{
	/*
	 * A debugging print method.
	 * Prints given move node with the format:
	 *"Command: command_info
	 * set i j prev_val next_val
	 * set i j prev_val next_val
	 * ...
	 *"
	 */
	move *m;/*move, not node*/
	if(move_node==NULL)
	{
		printf("The move DLL node is NULL!\n");
		return;
	}
	m = (move*)(move_node->data);
	if(m==NULL)
	{
		printf("The move pointer (data of node) is NULL!\n");
		return;
	}
	printf("Command: %s\n", m->command_info);
	print_set_list_backwards(m->set_list_end);
}
void append_new_move(char* command_info)
{
	/*
	 * Allocates memory to a new move with the data: (command_info, set_list_end = NULL),
	 * destroys all moves after recent_move, appends the new move to the move list (right after recent_move),
	 * and updates recent_move to point at the new move.
	 * On a memory allocation error prints the error message and terminates the program.
	 */
	move *new_move;
	char *cmdinfo;

	new_move = (move*)malloc(sizeof(move));
	check_alloc(new_move,"append_new_move");
	/*the string inside command_info is inside the heap,
	 *and because we want to use this string outside this function,
	 *we need to allocate memory for it on the heap.
	 *If we don't do that, the string will be removed or lost inside the stack after the function has returned,
	 *and we won't be able to access it.*/
	cmdinfo = (char*)malloc(sizeof(char)*strlen(command_info));
	check_alloc(cmdinfo,"append_new_move");
	strcpy(cmdinfo, command_info);/*from command_info(stack) to cmdinfo(heap)*/
	new_move->command_info = cmdinfo;
	new_move->set_list_end = NULL;
	/*printf(" Appending an empty move. recent move before appending:\n");debug*/
	/*print_move_node(recent_move);debug*/
	if(recent_move==NULL)
	{/*meaning we are right after a load move*/
		/*discarding all previous moves*/
		free_move_list(move_list_head);
		move_list_head = NULL;

		addAndMakeHead(&move_list_head, new_move);
		recent_move = move_list_head;
	}
	else
	{/*meaning we are right after a command that the user did*/
		addAndMakeHead(&recent_move, new_move);
	}
	/*printf(" recent move after appending the new one:\n");debug*/
	/*print_move_node(recent_move);debug*/
}

void revert_new_move(sudoku_board *board, STATUS s)
{
	/*
	 * Revert the action of appending a new move.
	 */
	doublyLinkedList new_move = recent_move;
	restore_prev_values_of_move(board, new_move,s);
	recent_move = new_move->prev;
	free_move_list(new_move);

	if(recent_move==NULL)
	{/*We removed the first command*/
		move_list_head = NULL; /*Without this line move_list_head will still point to new_move, which is freed*/
	}
}

void append_ijval_to_recent_move(int i, int j, int prev_val, int ne_val)
{
	/*
	 * Allocates memory to a new ijval structure with the data: (i,j,pre_val,ne_val),
	 * destroys all moves after set_list_end, appends the new ijval struct
	 * to the set list (right after set_list_end),
	 * and updates set_list_end to point at the new ijval struct.
	 * On a memory allocation error prints the error message and terminates the program.
	 */
	ijval *new_ijval;

	/*memory allocation*/
	new_ijval = (ijval*)malloc(sizeof(ijval));
	check_alloc(new_ijval,"append_ijval_to_move");
	/*assigning values to ijval structure*/
	new_ijval->i = i;
	new_ijval->j = j;
	new_ijval-> prev_val = prev_val;
	new_ijval->ne_val = ne_val;
	/*printf("Appending ijval:\n");debug*/
	/*print_ijval(new_ijval);debug*/
	/*the line below is ugly so the appending won't be on a local variable which will point to the same node*/
	addAndMakeHead(&(((move*)(recent_move->data))->set_list_end), new_ijval);
	/*printf("Recent move after appending:\n");debug*/
	/*print_move_node(recent_move);debug*/
}

void initialize_move_list()
{
	/*
	 * Makes recent_move and move_list_head point to NULL.
	 * The previous move list pointers are ignored (not freed, in case they will be needed).
	 */
	recent_move = NULL;
	move_list_head = NULL;
}

static void free_move(move *m)
{
	/*
	 * Frees the command_info string and the doubly linked list
	 * set_list_end and then frees the move structure itself.
	 */
	if(m!=NULL)
	{
		/*printf(" move node is not NULL.\n");*/
		/*printf("\tfreeing command_info %s\n",m->command_info);*/
		free(m->command_info);
		/*printf("\tfreeing set_list_end\n");*/
		destroyDoublyLinkedList_backwards(m->set_list_end);
		/*printf("\tfreeing m\n");*/
		free(m);
	}
}

void free_move_list(doublyLinkedList head_node)
{
	/*
	 * head is the head of the move list you want freed.
	 * The method frees all allocated memory inside the list from head and on:
	 * For each move - frees the set list, then frees the move structure.
	 * If this is not the first node, the previous nodes will remain untouched except now the last of them points to NULL.
	 */
	/*printf("Entered free_move_list:\n");*/
	if(head_node!=NULL)
	{
		/*printf(" node is not NULL.\n");*//*debug*/
		/*printf(" Starting to free all the next nodes:\n");*//*debug*/
		free_move_list(head_node->next);
		/*printf(" done freeing all the next nodes.\n");*//*debug*/
		if(head_node->prev!=NULL)
		{/*if there is a node before this node,
		  *cut this node from the previous nodes*/
			(head_node->prev)->next = NULL;
		}
		/*printf("Freeing the move structure...");*//*debug*/
		free_move((move*)head_node->data);
		/*printf("done\n");*//*debug*/
		/*printf("Freeing the DLL node...");*//*debug*/
		free(head_node);
		/*printf("done.\n");*//*debug*/
	}
	else
	{
		/*printf(" node is NULL.\n");*//*debug*/
	}
}

static void update_filled_cells(sudoku_board *board, int cur_val, int future_val)
{
	/*
	 * Updates board->filled_cells field to fit the change of the value:
	 * cur_val will become future_val.
	 */
	if(cur_val*future_val == 0 && !(cur_val==0 && future_val==0))
	{/*only one is empty*/
		if(future_val == 0)
		{/*filled now, will be empty*/
			board->filled_cells--;
		}
		else
		{/*empty now, will be filled*/
			board->filled_cells++;
		}
	}
	/*if both empty or both filled, we don't need to change filled_cells*/
}

void restore_prev_values_of_move(sudoku_board *board, doublyLinkedNode *move_node, STATUS s)
{
   /*
 	* Assigns all previous values in the move to the board's cells (undoing this move),
 	* and updates the errors.
 	* Does not change the recent_move pointer.
 	* Assumes board is initialized.
 	*/
	 doublyLinkedNode *end_set_node = ((move*)(move_node->data))->set_list_end;
	 ijval *ijvalp;
	 int i,j,pre_val;

	 if(end_set_node == NULL)
	 {/*nothing was changed (e.g. autofill without obvious cells)*/
		 return;
	 }
	 if(end_set_node->prev==NULL)
	 {/*if this is a single-set list*/
	 	ijvalp = (ijval*)(end_set_node->data);
	 	i = ijvalp->i;
		j = ijvalp->j;
		pre_val = ijvalp->prev_val;
		update_filled_cells(board, board->cells[i-1][j-1], pre_val);
		board->cells[i-1][j-1] = pre_val;
		update_err_changed_cell(board, i, j, s);
		return;
	 }
	 else
	 {/*if this is a multi-set move*/
		while(end_set_node!=NULL)
		{/*go over all the nodes, from last to first*/
			ijvalp = (ijval*)(end_set_node->data);
			i = ijvalp->i;
			j = ijvalp->j;
			pre_val=ijvalp->prev_val;
			update_filled_cells(board, board->cells[i-1][j-1], pre_val);
			board->cells[i-1][j-1] = pre_val;
			end_set_node = end_set_node->prev;
		}
		/*finished restoring from all nodes*/
		update_err_board(board, s);
		return;
	 }
}

void restore_next_values_of_move(sudoku_board *board, doublyLinkedNode *move_node, STATUS s)
{
   /*
	* Assigns all next values in the move to the board's cells (redoing this move),
	* and updates the errors.
	* Does not change the recent_move pointer.
	*/
	 doublyLinkedNode *set_node = ((move*)(move_node->data))->set_list_end;
	 ijval *ijvalp;
	 int i,j,ne_val;

	 if(set_node==NULL)
	 {/*if there weren't any sets in this move -> empty load*/
		 /*the board is already empty*/
		 return;
	 }
	 if(set_node->prev==NULL)
	 {/*if this is a single-set list*/
	 	ijvalp = (ijval*)(set_node->data);
	 	i = ijvalp->i;
		j = ijvalp->j;
		ne_val = ijvalp->ne_val;
		update_filled_cells(board, board->cells[i-1][j-1], ne_val);
		board->cells[i-1][j-1] = ne_val;
		update_err_changed_cell(board, i, j, s);
		return;
	 }
	 else
	 {/*if this is a multi-set move*/
		while(set_node!=NULL)
		{/*go over all the nodes, from last to first*/
			ijvalp = (ijval*)(set_node->data);
			i = ijvalp->i;
			j = ijvalp->j;
			ne_val = ijvalp->ne_val;
			update_filled_cells(board, board->cells[i-1][j-1], ne_val);
			board->cells[i-1][j-1] = ne_val;
			set_node = set_node->prev;
		}
		/*finished restoring from all nodes*/
		update_err_board(board, s);
		return;
	 }
}
