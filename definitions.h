#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

/*
 * This module contains all needed definitions for the Sudoku game.
 */

typedef int BOOL;

#define TRUE 	(1)
#define FALSE 	(0)

#define ERROR   (-1)

typedef int 	STATUS;

#define file_format (".txt") /*the valid format for puzzle files*/

#define EDIT (4)
#define SOLVE (3)
#define INIT (2)
#define PLAYING (1)
#define EXIT (0)

#define MEM_ALLOC_ERR (-1)

typedef struct sudoku_board_struct
{
	int length; /*the size of the board is length*length */
	int block_horizontal; /*the width of each block*/
	int block_vertical; /*the length of each block*/
	int **cells; /*contains filled values in the board*/
	int **solution; /*contains a solved board with the same filled values as cells*/
	int **fixed_or_error; /*contains whether the cells are fixed (TRUE/FALSE) or errors(2)*/
	int filled_cells; /*number of cells which are non empty*/
	int num_errors; /*Tells how many errors the boards has. If the board has 2 neighbor cells which
	*contain the same value the number of errors is 1. Every time a cell was set to have the same value as one
	*of its neighbors this parameter is increased by 1.*/
	BOOL isInit; /*TRUE if all the 2D arrays in board were allocated*/
}sudoku_board;

#endif /*__DEFINITIONS_H__*/
