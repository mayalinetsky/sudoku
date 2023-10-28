#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main_aux.h"
#include "parser.h"
#include "SPBufferset.h"
#include "definitions.h"

/*#define DEBUG *//*uncomment to switch to debug version of main*/

#ifndef DEBUG
/*
 * The main function of the Sudoku game.
 */
int main()
{
	STATUS current_status = INIT;
	sudoku_board *game_board = malloc(sizeof(sudoku_board));
	int seed = time(NULL);
	check_alloc(game_board,"main");
	game_board->isInit = FALSE;
	SP_BUFF_SET(); /*makes prints appear correctly*/
	srand(seed);
	printf("TOP OF THE MORNING TO YA LADIES AND GENTS my name is sudoku-console and welcome to the game!\n");
	while(current_status!=EXIT)
	{
		current_status = get_and_execute_command(game_board, current_status);
	}
	exit_game(game_board);
	return(0);
}

#else
SET_BUFF_SIZE();
int main()
{
	doublyLinkedList list1 = NULL, list2 = NULL;
	int fish[] = {1,2,3,4};
	SP_BUFF_SET();
	printf("DEBUG\n");
	push(&list1,fish);
	printf("%d\n",*(int*)(list1->data));
	push(&list1,fish+1);
	list2 = list1;
	printf("%d\n",*(int*)(list1->prev->data));
	printf("%d\n",*(int*)(list1->data));
	push(&list1,fish+2);
	printf("%d\n",*(int*)(list1->prev->prev->data));
	printf("%d\n",*(int*)(list1->prev->data));
	printf("%d\n",*(int*)(list1->data));
	printf("\n\n");
	list1 = list2->prev;
	addAndMakeHead(&list2,fish+3);/*This terminated the program because it attempts to free the data, which is fish.*/
	printf("%d\n",*(int*)(list2->data));
	printf("%d\n",*(int*)(list2->next->data));
	destroyDoublyLinkedList(list1);
	return 0;
}
#endif

