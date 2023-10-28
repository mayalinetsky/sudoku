#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "move_list.h"
#include "main_aux.h"
#include "solver.h"
#include "matrix.h"

/*
 *This module encapsulates the Sudoku puzzle game and the Sudoku board.
 *It will be used to print, store and manipulate the board, or when a property of the board
 *is needed, such as the board's stored solution, fixed cells, legal values for a cell and many more :)
 */

int mark_errors = 1; /*A global parameter with value of either 1(True) or 0(False) which determines whether errors in the Sudoku puzzle are displayed.*/

static void generate_fixed_cells_from_solution(sudoku_board *board, int **prev_cells)
{
	/*
	 * sets cells of board to be a randomized subset of the solution in board.
	 * randomly picks board->filled_cells cells to be filled and fixed ("hints").
	 * Updates the set list of recent_move keeping in mind the values of the cells
	 * before the generate command (prev_cells).
	 */
	int already_filled = 0; /*counter for the number of cells filled*/
	int length = board->length;
	int num_to_fill = board->filled_cells; /*number of cells needed to be filled*/
	int prev_value, fixed_state;
	int i,j;

	while(already_filled < num_to_fill)
	{
		i = get_index(length); /*random index*/
		j = get_index(length); /*random index*/
		if(fixed_or_error(board,i,j)!=1)
		{/*if cell is not fixed (but can be non-empty) adds it from solution to cells and makes it fixed*/
			board->cells[i-1][j-1] = get_solution(board,i,j);
			already_filled++;
			board->fixed_or_error[i-1][j-1] = 1;
			append_ijval_to_recent_move(i,j,get_matrix(prev_cells,i,j,length),get_solution(board,i,j));
		}
	}
	/*
	 * Checking if there is a filled cell in prev_cells that wasn't chosen to be fixed in the above code,
	 * meaning it will be empty on the new board.
	 */
	for(i=1; i<=length ;i++)
	{
		for(j=1; j<=length ;j++)
		{
			prev_value = get_matrix(prev_cells,i,j,length);
			fixed_state = get_matrix(board->fixed_or_error,i,j,length);
			if(prev_value!=0 && fixed_state==0)
			{/*non empty that didn't get fixed in the above code*/
				append_ijval_to_recent_move(i,j,prev_value,0);
			}
		}
	}

	/*printf("fixed cells: \n"); debug*/
	/*print_matrix(board->fixed_or_error,length);  debug*/
}

static int** find_empty_cells(sudoku_board *board, int num_empty_cells)
{
	/*
	 * Given a board and amount of empty cells, returns the first num_empty_cells found in board
	 * If there are less than that amount, the last cells of the array may be NULL.
	 * The format of the array is empty_cells_array[k] is the k-th doublet {i,j} containing the indices of the empty cells.
	 */
	int i,j,k = 0,length;
	int **empty_cells_array = malloc(num_empty_cells*sizeof(int*)); /*an array containing the locations of all empty cells.*/
	length = board->length;
	check_alloc(empty_cells_array,"find_empty_cells");
	for(i = 1; i <= length;i++)
	{
		for(j = 1; j <= length; j++)
		{
			if( get(board,i,j) == 0) /*found an empty cell*/
			{
				empty_cells_array[k] = malloc(2*sizeof(int));
				check_alloc(empty_cells_array[k],"find_empty_cells");
				empty_cells_array[k][0] = i; /*saves i*/
				empty_cells_array[k][1] = j; /*saves j*/
				k++; /*k is now the number of empty cells found*/
				if(k == num_empty_cells)
				{
					return empty_cells_array;
				}
			}
		}
	}
	return empty_cells_array;
}

void generate(sudoku_board *board, int num_to_fill, int num_fixed)
{
	/*
	 * Generates a puzzle by randomly filling num_to_fill empty cells on the current board with legal values,
	 * running ILP to solve the board, and then clearing all but (any) num_fixed random cells.
	 * Assumes num_to_fill and num_fixed are legal values and in the correct range, plus the board is not erroneous.
	 * The method prints the board if the command was executed successfully.
	 */
	int length = board->length;
	int num_previously_filled = board->filled_cells;
	int num_empty_cells = length*length-num_previously_filled; /*length^2 is the total amount of cells*/
	int **empty_cells_array;
	int i,j,k;
	int n,num_filled = 0;
	int val_to_fill;
	LINKEDLIST legal_values = NULL;
	int **prev_cells;/*the cells before the generate command*/
	int tries;
	const int MAX_ITERATIONS = 1000;
	BOOL successful = FALSE;
	char command_info[8+1+2+1+2+1];/*command_info for the move structure. "generate"+space+(2 digits)+space+(2 digits)+\0*/

	empty_cells_array = find_empty_cells(board, num_empty_cells);
	prev_cells = make_matrix(length);
	copy_matrix(board->cells,prev_cells,length); /*storing a copy to revert to in case of an error*/

	sprintf(command_info, "generate %d %d", num_to_fill, num_fixed);
	append_new_move(command_info);

	for(tries = 0; tries < MAX_ITERATIONS; tries++)
	{
		num_filled = 0;
		/*filling empty cells*/
		while ( num_filled < num_to_fill )
		{
			k = get_index(num_empty_cells) - 1; /*get_index returns a value >=1, we need >=0*/
			/*extract empty cell location*/
			i = empty_cells_array[k][0];
			j = empty_cells_array[k][1];
			if( get(board,i,j) == 0) /*if this cell hasn't already been filled by previous iterations*/
			{
				legal_values = get_legal_values(board,i,j,FALSE); /*possible assignments*/
				/*choosing a random legal value*/
				n = getLength(legal_values);
				if(n==0)
				{
					copy_matrix(prev_cells,board->cells,length); /*can't go further, reverting and then trying again*/
					break; /*no need to free legal values since it is already NULL*/
				}
				k = get_index(n) - 1;
				val_to_fill = getLinkedListByIndex(legal_values,k)->data;
				/*not using set because the value is legal and the cell is empty so no extra checks and prints needed*/
				board->cells[i-1][j-1]=val_to_fill;
				num_filled++;

				/*cleaning legal_values*/
				destroyLinkedList(legal_values);
				legal_values = NULL;
			}
		}

		/*
		 * if we have reached this section of the code,
		 * we have filled num_to_fill cells successfully
		 * or
		 * there is a cell with no legal values -> no solution to the board (successful will be FALSE)
		 */
		if( num_filled < num_to_fill )
		{/*not enough cells, we were cut in the middle, try again*/
			continue;
		}

		successful = solve_board(board); /*solve_board takes care of reverting the solution to its original state*/
		if(successful==TRUE)
		{/*if "num_to_fill" empty cells were filled and there is a solution*/
			break;
		}
		/*unsuccessful or error*/
		copy_matrix(prev_cells,board->cells,length);
	}
	if(successful==FALSE)
	{
		print_too_many_iterations("generate_board");
		/*remove the new move we added*/
		revert_new_move(board, EDIT);
		return;
	}

	/*free empty_cells_array*/
	for(k = 0; k < num_empty_cells; k++)
	{
		free(empty_cells_array[k]);
	}
	free(empty_cells_array);

	 /*for generate_fixed_cells_from_solution*/
	board->filled_cells = num_fixed; /*update the amount of cells to be num_fixed*/


	free_matrix(board->fixed_or_error,length); /*these 2 commands set all cells of the matrix to 0*/
	board->fixed_or_error = make_matrix(length);

	free_matrix(board->cells,length); /*these 2 commands set all cells of the matrix to 0*/
	board->cells = make_matrix(length);

	generate_fixed_cells_from_solution(board, prev_cells);/*updates board->cells AND the set list of recent move*/
	free_matrix(prev_cells,length);
	/*If this line is reached, the board definitely changed*/
	print_board(board, EDIT);/*generate is only available in Edit mode*/
	return;
}

int get(sudoku_board *board, int i, int j)
{
	/*
	 * Gets the (i-1,j-1) cell's value of the board.
	 * returns: the cells value if valid. ERROR if failed, 0 if cell is empty.
	 */
	return get_matrix((board->cells),i,j,(board->length));
}

BOOL fixed_or_error(sudoku_board *board, int i,int j)
{
	/*
	 * Gets the (i-1,j-1) fixation/error state.
	 * returns: 2 if the cell is erroneous, 1 if the cell is fixed, 0 if neither. ERROR if failed.
	 */
	return get_matrix((board->fixed_or_error),i,j,(board->length));
}
static void update_err_row(sudoku_board *board,int row_num, STATUS s)
{
	/*
	 * Marks all errors caused by repeating digits in row row_num (1 to length) as errors, and updates board->num_erros accordingly.
	 * Assumes row_num is legal and that the board is initialized
	 * Non erroneous cells will be updated as well
	 */
	int i, digit, fixerr ,n;
	int length = board->length;
	LINKEDLIST pos;
	/*an array such that arr[j] contains a list of all column numbers in which the j+1 digit occurs*/
	LINKEDLIST* arr = calloc(length, sizeof(LINKEDLIST));
	check_alloc(arr, "update_err_row");

	for(i = 1; i <= length; i++)
	{
		digit = board->cells[i-1][row_num-1];
		if(digit>0)/*we don't care about empty cells*/
		{
			appendToLinkedList(arr+digit-1,i);/*adds the index i to the linked list of the corresponding digit*/
		}
		else/*empty cell*/
		{
			if(fixed_or_error(board,i,row_num)==2) /*empty cells should not be erroneous*/
			{
				board->fixed_or_error[i-1][row_num-1] = 0;
				board->num_errors--;
			}
		}
	}

	for(digit = 1; digit <= length; digit++)
	{
		pos = arr[digit-1];
		n = getLength(pos);
		if(n>1) /*multiple occurrences of a digit is an error for each*/
		{
			while(pos!=NULL)
			{
				i = pos->data;
				fixerr = fixed_or_error(board,i,row_num);
				if(fixerr == 0 || (fixerr == 1 && s == EDIT)) /*there is an unmarked error on a non-fixed cell, mark it*/
				{
					board->fixed_or_error[i-1][row_num-1] = 2;
					board->num_errors++;
				}
				pos = pos->next;
			}

		}
		else if (n==1)
		{
			i = pos->data;
			/*we still need to make sure the value is indeed legal because it might be erroneous due to column or block*/
			fixerr = fixed_or_error(board,i,row_num);
			if((fixerr==2) && is_legal(board,i,row_num,digit,FALSE)) /*cell marked as erroneous but isn't*/
			{
				board->fixed_or_error[i-1][row_num-1] = 0;
				board->num_errors--;
			}
		}
		destroyLinkedList(arr[digit-1]);
	}
	free(arr);
}

static void update_err_column(sudoku_board *board,int column_num, STATUS s)
{
	/*
	 * Marks all errors caused by repeating digits in column column_num (1 to length) as errors, and updates board->num_erros accordingly.
	 * Assumes row_num is legal and that the board is initialized
	 * Non erroneous cells will be updated as well
	 */
	int j, digit, fixerr ,n;
	int length = board->length;
	LINKEDLIST pos;
	/*an array such that arr[i] contains a list of all row numbers in which the i+1 digit occurs*/
	LINKEDLIST* arr = calloc(length, sizeof(LINKEDLIST));
	check_alloc(arr,"update_err_column");

	for(j = 1; j <= length; j++)
	{
		digit = board->cells[column_num-1][j-1];
		if(digit>0)/*we don't care about empty cells*/
		{
			appendToLinkedList(arr+digit-1,j);/*adds the index j to the linked list of the corresponding digit*/
		}
		else/*empty cell*/
		{
			if(fixed_or_error(board,column_num,j)==2) /*empty cells should not be erroneous*/
			{
				board->fixed_or_error[column_num-1][j-1] = 0;
				board->num_errors--;
			}
		}
	}

	for(digit = 1; digit <= length; digit++)
	{
		pos = arr[digit-1];
		n = getLength(pos);
		if(n>1) /*multiple occurrences of a digit is an error for each*/
		{
			while(pos!=NULL)
			{
				j = pos->data;
				fixerr = fixed_or_error(board,column_num,j);
				if(fixerr == 0 || (fixerr == 1 && s == EDIT)) /*there is an unmarked error on a non-fixed cell, mark it*/
				{
					board->fixed_or_error[column_num-1][j-1] = 2;
					board->num_errors++;
				}
				pos = pos->next;
			}

		}
		else if (n==1)
		{
			j = pos->data;
			/*we still need to make sure the value is indeed legal because it might be erroneous due to row or block*/
			fixerr = fixed_or_error(board,column_num,j);
			if((fixerr==2) && is_legal(board,column_num,j,digit,FALSE)) /*cell marked as erroneous but isn't*/
			{
				board->fixed_or_error[column_num-1][j-1] = 0;
				board->num_errors--;
			}
		}
		destroyLinkedList(arr[digit-1]);
	}
	free(arr);

}

static void update_err_block(sudoku_board *board,int column_num ,int row_num, STATUS s)
{
	/*
	 * Marks all errors caused by repeating digits in block that has at [column_num,row_num] as errors, and updates board->num_erros accordingly.
	 * Assumes row_num is legal and that the board is initialized
	 * Non erroneous cells will be updated as well
	 */
	int I,J,i,j,k, digit, fixerr ,n;
	int length = board->length;
	int hor = board->block_horizontal;
	int ver = board->block_vertical;

	LINKEDLIST pos;
	/*an array such that arr[l] contains a list of all row numbers in which the l+1 digit occurs*/
	LINKEDLIST* arr = calloc(length, sizeof(LINKEDLIST));

	I = ((column_num-1)/hor)*hor+1; /*indices of beginning of th block*/
	J = ((row_num-1)/ver)*ver+1;

	check_alloc(arr,"update_err_row");

	for(k = 0; k < length; k++)
	{
		i = I+k%hor;
		j = J + k/hor;
		/*printf("%d %d\n",i,j);*/
		digit = board->cells[i-1][j-1];
		if(digit>0)/*we don't care about empty cells*/
		{
			appendToLinkedList(arr+digit-1,k);/*adds the index j to the linked list of the corresponding digit*/
		}
		else/*empty cell*/
		{
			if(fixed_or_error(board,i,j)==2) /*empty cells should not be erroneous*/
			{
				board->fixed_or_error[i-1][j-1] = 0;
				board->num_errors--;
			}
		}
	}

	for(digit = 1; digit <= length; digit++)
	{
		pos = arr[digit-1];
		n = getLength(pos);
		if(n>1) /*multiple occurrences of a digit is an error for each*/
		{
			while(pos!=NULL)
			{
				k = pos->data;
				i = I+k%hor;
				j = J + k/hor;
				fixerr = fixed_or_error(board,i,j);
				if(fixerr == 0 || (fixerr == 1 && s == EDIT)) /*there is an unmarked error on a non-fixed cell, mark it*/
				{
					board->fixed_or_error[i-1][j-1] = 2;
					board->num_errors++;
				}
				pos = pos->next;
			}

		}
		else if (n==1)
		{
			k = pos->data;
			i = I+k%hor;
			j = J + k/ver;
			/*we still need to make sure the value is indeed legal because it might be erroneous due to row or block*/
			fixerr = fixed_or_error(board,i,j);
			if((fixerr==2) && is_legal(board,i,j,digit,FALSE)) /*cell marked as erroneous but isn't*/
			{
				board->fixed_or_error[i-1][j-1] = 0;
				board->num_errors--;
			}
		}
		destroyLinkedList(arr[digit-1]);
	}
	free(arr);

}

void update_err_changed_cell(sudoku_board *board, int i, int j,STATUS s)
{
	/*
	 * updates all erroneous values that might have changed due to updating the (i-1,j-1) cell
	 */
	update_err_row(board,j,s);
	update_err_column(board,i,s);
	update_err_block(board,i,j,s);
}

BOOL set(sudoku_board *board, int i, int j, int value, STATUS status)
{
	/*
	 * Sets the (i-1,j-1) cell to value. if value is 0, clears the cell.
	 * returns TRUE if the last cell on the board was filled, FALSE otherwise.
	 * Assumes board is not NULL, i,j are in the correct range.
	 * Assumes that if called in solve mode, the cell is not fixed.
	 * An erroneous value (equal to one of the neighbors) is allowed.
	 * updates:
	 * board.filled_cells.
	 * board.fixed_or_error when an illegal value was set.
	 * board.num_errors to the new number of errors.
	 * recent_move to this set move and destroys any move that was after recent_move before this command was called.
	 */
	int **M = board->cells;
	int length = board->length;
	BOOL last_cell_filled;/*, legal_i_j;*/
	int last_value = get(board,i,j);
	char command_info[13];/*data for the move list. 13 characters including "\0" and assuming i, j, value are 2 digits or less each*/

	/*legal_i_j = is_legal(board,i,j,value,FALSE);*/
	if(value==0)
	{/*we know last_value!=value meaning the cell is cleared*/
		board->filled_cells--;
	}
	else
	{/*the user did not clear the cell*/
		if(last_value==0)
		{
			board->filled_cells++;
		}
	}

	M[i-1][j-1] = value;

	update_err_changed_cell(board, i, j, status);
	print_board(board,status);

	sprintf(command_info, "set %d %d %d", i, j, value);
	append_new_move(command_info);
	append_ijval_to_recent_move(i,j, last_value, value);

	last_cell_filled = ( (board->filled_cells) == (length*length) );
	return last_cell_filled;
}

static void print_cell(sudoku_board *board, int row_num, int col_num, STATUS status)
{
	/*
	 * Helper method for print_row.
	 * Prints the cell in row number "row_num" and column number "col_num" of board.
	 * The cell is represented with 4 characters:
	 * 		A space character.
	 * 		Two digits for the cell value (two spaces for a blank cell).
	 * 		A dot '.' for a fixed cell, an '*' for an erroneous cell when in edit mode
	 * 			or the "mark_errors" parameter is 1, or space otherwise.
	 * NOTE: this method prints only 4 characters. W\O any spaces before of after.
	 */
	int value = get(board, col_num, row_num);

	if(value==0)
	{/*if cell is empty*/
		printf("    ");
	}
	else
	{
		printf(" %2d",value);
		if(fixed_or_error(board, col_num, row_num)==1 && status==SOLVE)
		{/*if cell is fixed*/
			printf(".");
		}
		else if((fixed_or_error(board, col_num, row_num)==2)&&(status==EDIT || mark_errors==1))
		{/*if the cell is problematic and we are in edit mode or we need to display the errors*/
			printf("*");
		}
		else
		{
			printf(" ");
		}
	}

}

static void print_row(sudoku_board *board, int row_num, STATUS status)
{
	/*
	 * Helper method for print_board.
	 * row_num can be 1,...,board->length
	 * Prints the row number "row_num" in board (INCLUDING "\n") using this format:
	 * 	a pipe '|' starts and ends each row.
	 * 	each cell is represented with 4 characters:
	 * 		A space character.
	 * 		Two digits for the cell value (two spaces for a blank cell).
	 * 		A dot '.' for a fixed cell, an '*' for an erroneous cell when in edit mode
	 * 			or the "mark_errors" parameter is 1, or space otherwise.
	 * 	a pipe '|' separates each set of board->block_horizontal digits.
	 */
	char pipe = '|';
	int cur_col = 1; /*the index of the column*/

	printf("%c",pipe);
	while(cur_col <= (board->length))/*while there are cells to print*/
	{
		print_cell(board, row_num, cur_col, status);
		if(cur_col%(board->block_horizontal)==0)
		{/*if the cell is the end of a block, add a pipe*/
			printf("%c",pipe);
		}
		cur_col+=1;
	}
	printf("\n");
}

static char* generate_separator_row(sudoku_board *board)
{
	/*
	 * Generates the separator row between blocks for print_board.
	 * Uses malloc. User's responsibility to use free() afterwards.
	 * Returns the pointer to the separator row. NULL if encountered a memory problem.
	 */
	int C = 4; /*Number of chars per cell*/
	int M = board->block_vertical; /*Number of blocks per line*/
	int length = (board->length);/*Number of cells per line*/
	int size_sep = C*length+M+3;/*size of separator row is C*length+M+1 dashes plus '\n' plus '\0'*/
	char *separator_row = (char*)malloc((size_sep)*sizeof(char));
	int i=0; /* index for the loop */
	if(separator_row==NULL)
	{
		return(NULL);
	}
	else
	{
		for(; i<size_sep-2; i++)/* -2 for '\n' and '\0' */
		{
			separator_row[i] = '-';
		}
		separator_row[i] = '\n';
		separator_row[i+1] = '\0';
		return(separator_row);
	}
}

void print_board(sudoku_board *board, STATUS status)
{
	/*
	 * Prints the board according to page 5 in the pdf.
	 * Exits game using exit_game and exit(0) if a memory problem was encountered.
	 * Frees separator_row.
	 */
	char *separator_row;
	int cur_row = 1;

	separator_row = generate_separator_row(board);
	check_alloc(separator_row, "print_board");


	printf("%s", separator_row);
	while(cur_row <= (board->length))/*while there are rows to print*/
	{
		print_row(board, cur_row, status);
		if(cur_row%(board->block_vertical)==0)
		{
			printf("%s", separator_row);
		}
		cur_row+=1;
	}
	free(separator_row);
}

static LINKEDLIST get_block(sudoku_board *board, int **M, int i, int j)
{
	/*
	 * returns a LINKEDLIST of all values in the block containing the (i-1,j-1) cell in board, excluding that cell.
	 * If board is NULL returns NULL.
	 * Assumes board is initialized.
	 */
	int length,I,J, vert,hort, temp;
	int l,m; /*loop indices*/
	LINKEDLIST block = NULL; /*the return-to-be value*/
	if(board == NULL)
	{
		printf("get_block NULL\n");
		return NULL;
	}

	length = board->length;

	hort = board->block_horizontal;
	vert = board->block_vertical;

	I = (i-1)/(hort); /*the vertical coordinate representing the block (there are block_horizontal of those)*/
	J = (j-1)/(vert); /*the horizontal coordinate representing the block (there are block_vertical of those)*/
	for(l = 1; l <= vert; l++)
	{
		for(m = 1; m <= hort; m++)
		{
			if( (m+I*hort == i) && (l+J*vert == j) )
			{
				continue;
			}
			temp = get_matrix(M,m+I*hort,l+J*vert,length);
			if(temp!=0)
			{
				appendToLinkedList(&block, temp); /*adds all cells in the block to the return list "block"*/
			}
		}
	}
	return block;
}

static LINKEDLIST get_column(sudoku_board *board, int **M, int i, int row)
{
	/*
	 * returns a LINKEDLIST of all values in i-1 row in board, excluding the one at row 'row' .
	 * If board is NULL returns NULL.
	 * Assumes board is initialized.
	 */
	int j, length, temp;
	LINKEDLIST column = NULL;
	if(board == NULL)
	{
		return NULL;
	}

	length = board->length;

	for(j = 1; j <= length; j++)
	{
		if(j != row)
		{
			temp = get_matrix(M, i, j,length);
			if(temp!=0)
			{
				appendToLinkedList(&column, temp);
			}
		}
	}
	return column;
}

static LINKEDLIST get_row(sudoku_board *board,int **M, int column ,int j)
{
	/*
	 * returns a LINKEDLIST of all values in j-1 row in board, excluding the one at column 'column' .
	 * If board is NULL returns NULL.
	 * Assumes board is initialized.
	 */
	int i, length, temp;
	LINKEDLIST row = NULL;
	if(board == NULL)
	{
		return NULL;
	}

	length = board->length;
	for(i = 1; i <= length; i++)
	{
		if(i != column)
		{
			temp = get_matrix(M, i, j,length);
			if(temp!=0)
			{
				appendToLinkedList(&row, temp);
			}
		}
	}
	return row;
}

static LINKEDLIST get_neighbors(sudoku_board *board, int **M, int i, int j)
{
	/*
	 * returns a linked list of all neighbors in the j row, i column of M, and the block containing the (i-1,j-1) element.
	 * if one of the values given is illegal, returns an empty list.
	 * Need to not include the said cell.
	 */
	LINKEDLIST row,column,block;
	row = get_row(board, M ,i, j);
	column = get_column(board, M, i, j);
	block = get_block(board, M, i, j);
	linkedListUnion(&row,column); /* "row += column" */
	linkedListUnion(&block,row); /* "block += row" */
	return block;
}

BOOL is_legal(sudoku_board *board, int i, int j, int value,BOOL sol)
{
	/*
	 * If sol is FALSE:
	 * Return True iff "value" is a legal value for column i row j of board (meaning it is different from all neighbors).
	 * If sol is TRUE:
	 * Return True iff "value" is a legal value for column i row j of board.solution (meaning it is different from all neighbors).
	 */
	LINKEDLIST neighbors;
	BOOL result;
	if(sol)
	{
		neighbors = get_neighbors(board,board->solution, i, j);
	}
	else
	{
		neighbors = get_neighbors(board,board->cells, i, j);
	}
	result = (searchLinkedList(neighbors,value)==NULL);
	destroyLinkedList(neighbors);

	return result;
}

LINKEDLIST get_legal_values(sudoku_board *board, int i, int j,BOOL sol)
{
	/*
	 * Returns a LINKEDLIST of legal values for the cell at row i-1 and column j-1.
	 */
	LINKEDLIST neighbors;
	LINKEDLIST result = NULL;
	int k;
	/*printf("getting neighbors\n");*/ /*debug*/
	if(sol)
	{
		neighbors = get_neighbors(board,board->solution, i, j);
	}
	else
	{
		neighbors = get_neighbors(board,board->cells, i, j);
	}
	/*printf("complete\n");*/ /*debug*/
	for(k=1; k<=board->length; k++)
	{
		/*printf("entering searchLinkedList\n");*/
		if(searchLinkedList(neighbors,k)==NULL)/*k is a legal value*/
		{
			/*printf("legal. entering append:\n");*/
			appendToLinkedList(&result,k);
			/*printf("complete\n");*/
		}
	}
	return result;
}

BOOL validate(sudoku_board *board)
{
	/*
	 * Validates that the current state of the board is solvable using ILP.
	 * Returns and prints if validation passed(TRUE) or failed(FALSE).
	 * If validation passed, updates the stored solution of board.
	 * Assumes the mode is Edit or Solve and that the board is not erroneous.
	 */
	BOOL solution_found = solve_board(board);
	if(solution_found==ERROR)
	{/*the solver printed the error*/
		return(ERROR);
	}
	if(solution_found==TRUE)
	{
		printf("Validation passed: board is solvable.\n");
		return(TRUE);
	}
	else
	{
		printf("Validation failed: board is unsolvable.\n");
		return(FALSE);
	}
}

void hint(sudoku_board *board, int i,int j)
{
	/*
	 * Runs ILP to solve the board. If the board is unsolvable prints an error message.
	 * Else, prints to the user the value at column i and row j of the solution.
	 * Assumes the mode is Solve, the board is not erroneous,
	 * cell <i,j> is not fixed and doesn't contain a value.
	 */
	BOOL boardSolvable = solve_board(board);
	int hint_digit;
	if(boardSolvable==ERROR)
	{/*The solver printed the error*/
		return;
	}
	if(boardSolvable==FALSE)
	{
		printf("Error: Board is unsolvable, can't hint.\n");
		return;
	}
	hint_digit = get_solution(board,i,j);
	printf("Hint: set cell to %d\n",hint_digit);
	return;
}

int get_solution(sudoku_board *board, int i, int j)
{
	/*
	 * returns the value of the i,j cell in solution.
	 * 1<=i,j<=length and board is initialized are assumed.
	 * If an error occurs or values are illegal, returns ERROR.
	 */
	return get_matrix(board->solution,i,j,board->length);
}

void set_mark_errors(int x)
{
	/*
	 * Sets the global parameter "mark_errors" to x.
	 * Assumes x is 1 or 0.
	 */
	mark_errors = x;
}

static double* get_high_scores_for_cell(double* scores3Dmat,int length,int i,int j,double X)
{
	/*
	 * Inputs: scores3Dmat is a flattened 3D matrix of scores given from the LP solution.
	 * 		   length is the board length.
	 * 		   i,j is the column number and row number of the wanted cell.
	 * 		   X is the threshold for the scores.
	 * Output: an allocated array 'scores' of doubles. Use free() when done using it.
	 * 		   Index 0 contains the last index belonging to the array (as a double).
	 * 		   From there, each odd index (1,3,5..) contains the value
	 * 		   and the following even index contains the score. All scores are X or higher.
	 * 		   In any way, 'scores' will not contain values with score 0.
	 * 		   If there are no values with scores of X (and not zero) or greater for this cell,
	 * 		   returns NULL.
	 */
	int val_count = 0;
	int k, target_indx;
	double score;
	double* wanted_scores;

	/*First count how many values have wanted scores:*/

	for(k=1; k<=length ; k++)
	{
		score = scores3Dmat[get_3d_index(i,j,k,length)];
		if(score>=X && score>0)
		{
			val_count++;
		}
	}
	/*printf("val_count = %d, arrlen = %d\n",val_count,2*val_count+1); debug*/
	if(val_count==0)
	{/*There are no values with scores of X or greater for this cell*/
		/*printf("NULL scores for cell %d %d\n",i,j); debug*/
		return NULL;
	}
	wanted_scores = (double*)malloc((1+val_count*2)*sizeof(double));
	check_alloc(wanted_scores,"get_high_scores_for_cell");
	wanted_scores[0]=val_count*2;
	k=1;/*value for the cell*/
	target_indx=1;/*index of val*/
	while(target_indx<=wanted_scores[0])
	{
		/*printf("value = %d, target_indx = %d\n", k,target_indx); debug*/
		score = scores3Dmat[get_3d_index(i,j,k,length)];
		if(score>=X && score!=0)
		{
			/*printf("\t%d is a wanted value with a score %.4f\n",k,score); debug*/
			wanted_scores[target_indx] = k;
			/*printf("\t\t %.4f\n",wanted_scores[target_indx]); debug*/
			wanted_scores[target_indx+1] = score;
			/*printf("\t\t %.4f\n",wanted_scores[target_indx+1]); debug*/
			target_indx+=2;
		}
		k++;
	}
	return wanted_scores;
}

static int choose_val_acc_score(double* cell_high_scores)
{
	/*
	 * Gets the scores array returned from 'get_high_scores_for_cell',
	 * and returns a random value according to the scores (a score of 0.6 has double the chance of 0.3).
	 */
	int *vals, *freq, count, indx=1, rand_val;
	double *scores = NULL;
	double minScore;
	count = (int)(cell_high_scores[0]/2);
	vals = (int*)malloc(count*sizeof(int));
	freq = (int*)malloc(count*sizeof(int));
	scores = (double*)malloc(count*sizeof(double));
	check_alloc(vals,"choose_val_acc_score");
	check_alloc(freq,"choose_val_acc_score");

	/*Ready the vals and freq arrays for the utility functions*/
	while(indx+1<=cell_high_scores[0])
	{
		vals[(indx-1)/2]=cell_high_scores[indx];/*indx is the index of the value in the array*/
		scores[(indx-1)/2]=cell_high_scores[indx+1];/*indx+1 contain the score of the above value*/
		indx+=2;
	}

	minScore = get_min(scores, count);

	for(indx=0; indx<count; indx++)
	{
		if(minScore==0)
		{
			/*printf("lsngslrjkgn the minimum is zerooo!\n"); debug*/
			exit(ERROR);
		}
		freq[indx] = (int)(100*scores[indx]/minScore);/*times 100 just to spread the frequencies*/
	}
	rand_val = myRand(vals, freq, count);
	free(scores);
	free(vals);
	free(freq);
	return rand_val;
}
BOOL guess(sudoku_board *board, float X)
{
	/*
	 * Guesses a solution to the current board using LP with threshold X.
	 * Fills all cell values with a score of X or greater. If several values hold for the same cell,
	 * randomly chooses one according to the score(a score of 0.6 has double the chance of 0.3).
	 * Doesn't fill illegal values created along the way.
	 * Prints the board when finished.
	 * Returns whether the last cell was filled.
	 * Assumes the mode is Solve, X is in the right range and the board is not erroneous.
	 */
	int i,j,prev_val,ne_val,length; /*i is column number, j is row number. both starting from 1.*/
	char command_info[5+1+40+1];/*command info for the move structure. "guess"+space+(float in base 10)+\0*/
	double* scores3Dmat; /*a flattened 3D matrix of the LP solution to the board*/
	double* cell_high_scores; /*an array of the values and scores of a cell*/
	BOOL board_has_changed = FALSE;

	/*creating a new move*/
	sprintf(command_info, "guess %f", X);
	append_new_move(command_info);

	/*Getting the scores from LP*/
	scores3Dmat = get_LP_scores(board);
	if(scores3Dmat==NULL)
	{/*the board is not solvable or Gurobi encountered an error*/
		revert_new_move(board, SOLVE);
		if(get_error_status()==TRUE)
		{/*encountered an error. Gurobi printed the error message*/
			return ERROR;
		}
		print_NULL_scores();
		return FALSE;
	}

	length = board->length;
	for(i=1 ; i<=length ; i++)
	{/*for the column i*/
		for(j=1 ; j<=length ; j++)
		{/*for the row j*/
			if((prev_val=get(board,i,j))!=0)
			{/*If this is not an empty cell*/
				continue;
			}
			cell_high_scores = get_high_scores_for_cell(scores3Dmat,length,i,j,X);
			if(cell_high_scores==NULL)
			{/*There are no values with scores of X or greater for this cell*/
				continue;
			}
			else
			{/*There is at least one wanted value*/
				ne_val = choose_val_acc_score(cell_high_scores);
				if(is_legal(board,i,j,ne_val,FALSE)==TRUE)
				{/*don't fill illegal values created along the way*/
					(board->cells)[i-1][j-1] = ne_val;
					board->filled_cells++;
					append_ijval_to_recent_move(i,j,prev_val,ne_val);
					board_has_changed = TRUE;
				}
				free(cell_high_scores);
			}
		}
	}

	free(scores3Dmat);
	if(board_has_changed==TRUE)
	{/*this check is to save time in case the board did not change*/
		update_err_board(board, SOLVE);
	}
	else
	{/*Just to tell the user*/
		print_no_change_in_guess(X);
	}
	print_board(board, SOLVE);/*guess is only available in Solve mode*/
	return board->filled_cells==length*length;
}

BOOL undo(sudoku_board *board, STATUS s)
{
	/*
	 * Undo the most recent move done by the user.
	 * Sets the current move pointer to the previous move and updates the board accordingly.
	 * This does not add or remove any item to/from the list.
	 * If there are no moves to undo, Returns TRUE. Returns FALSE otherwise.
	 * If there is a move to undo, clearly prints to the user the change that was made.
	 */
	move *the_move;/*move pointer, not DLL node*/
	if(recent_move==NULL)
	{/*meaning we are right after a load move*/
		return(TRUE);
	}
	else
	{/*meaning we are right after a command the user made*/
		the_move = (move*)(recent_move->data);
		printf("Undoing command: %s\n",(the_move->command_info));
		restore_prev_values_of_move(board, recent_move , s);
		recent_move = recent_move->prev;
		print_board(board, s);
		return(FALSE);
	}
}


BOOL redo(sudoku_board *board, STATUS s)
{
	/*
	 * Redo previous move done by the user.
	 * Sets the current move pointer to the next move and updates the board accordingly.
	 * This does not add or remove any item to/from the list.
	 * If there are no moves to redo, Returns TRUE. Returns FALSE otherwise.
	 * If there is a move to redo, clearly prints to the user the change that was made.
	 */
	doublyLinkedNode *next_move_node;
	move *next_move;
	if(recent_move==NULL)
	{/*meaning we are right after a load move*/
		if(move_list_head==NULL)
		{/*meaning there are no other moves but load*/
			return(TRUE);
		}
		else
		{/*meaning there are other moves but load*/
			next_move_node = move_list_head;
			goto RedoNextMove;
		}
	}
	else
	{/*meaning we are right after a command the user made*/
		next_move_node = recent_move->next;
		if(next_move_node == NULL)
		{/*if there are no moves after this one*/
			return(TRUE);
		}
		else
		{/*if there is a move after this one*/
			goto RedoNextMove;
		}
	}
	RedoNextMove:
				next_move = (move*)next_move_node->data;
				printf("Redoing command: %s\n",next_move->command_info);
				restore_next_values_of_move(board, next_move_node, s);
				recent_move = next_move_node;
				print_board(board, s);
				return(FALSE);
}

void save(sudoku_board *board, STATUS s, FILE *f)
{
	/*
	 * Saves the current game board to the specified file f, according to the file format in page 13 in the pdf.
	 * Assumes f points to an existing valid file, the mode is right and the board is valid.
	 * In Edit mode, cells containing values are marked as "fixed" in the saved file.
	 * Prints that the board was successfully saved.
	 */
	int i,j,length, val;
	length = board->length;
	fprintf(f,"%d %d\n",board->block_vertical,board->block_horizontal);
	for(j = 1; j <= length; j++)
	{
		for(i = 1; i <= length; i++)
		{
			val = get(board,i,j);
			fprintf(f,"%d",val);
			if(fixed_or_error(board,i,j) == 1 || (s==EDIT && val!=0))
			{
				fwrite(". ",sizeof(char),2,f);
			}
			else if(i<length) /*don't write a space at the end of the line*/
			{
				fwrite(" ",sizeof(char),1,f);
			}
		}
		fwrite("\n",sizeof(char),1,f);
	}
	printf("File saved successfully.\n");
	return;
}

void guess_hint(sudoku_board *board, int x, int y)
{
	/*
	 * Runs LP to solve the board. If the board is unsolvable prints an error message.
	 * Else, prints to the user all legal values for column x and row y of the board,
	 * and their score (prints only values with scores greater than 0).
	 * Assumes the mode is Solve, the board is not erroneous,
	 * cell <x,y> is not fixed and doesn't contain a value.
	 */
	int length, i, val;
	double* scores3Dmat; /*a flattened 3D matrix of the LP solution to the board*/
	double* cell_high_scores; /*an array of the values and scores of a cell*/
	double score;

	/*Getting the scores from LP*/
	scores3Dmat = get_LP_scores(board);
	if(scores3Dmat==NULL)
	{/*the board is not solvable or Gurobi encountered an error*/
		if(get_error_status()==TRUE)
		{/*encountered an error. Gurobi printed the error message*/
			return;
		}
		print_NULL_scores();
		return;
	}

	length = board->length;
	cell_high_scores = get_high_scores_for_cell(scores3Dmat,length,x,y,0);
	free(scores3Dmat);
	if(cell_high_scores==NULL)
	{/*There are no values with scores greater than 0 for this cell*/
		print_no_wanted_scores();
	}
	else
	{/*There is at least one wanted value -> print them to the user*/
		printf("Legal values for the cell <%d,%d>:\n\tValue\t(score)\n", x,y);
		for(i=1 ; i<(int)(cell_high_scores[0]); i+=2)
		{
			val = (int)cell_high_scores[i];
			score = cell_high_scores[i+1];
			printf("\t%d\t(%.4f)\n",val, score);
		}
		free(cell_high_scores);
	}

}

void num_solutions(sudoku_board *board)
{
	/*
	 * Runs an exhaustive backtracking algorithm for the current board to find how many solutions
	 * there are, and then prints the result.
	 */
	printf("Exhaustive Backtracking complete. %d solutions found.\n",exhaustive_backtracking(board));
}

static void init_board(sudoku_board *board, int block_horizontal, int block_vertical)
{
	/*
	 * Initializes the board state to be workable.
	 * Assumes board is not NULL. If isInit, frees the memory.
	 */
	int length = block_horizontal * block_vertical;
	int prev_length = board->length;
	if(board->isInit)
	{
		/*printf("freeing previous matrices\n");*/
		free_matrix(board->cells,prev_length);
		free_matrix(board->solution,prev_length);
		free_matrix(board->fixed_or_error,prev_length);
		/*printf("done\n");*/
	}
	board->block_horizontal = block_horizontal;
	board->block_vertical = block_vertical;
	board->length = length;
	board->cells = make_matrix(length);
	board->solution = make_matrix(length);
	board->fixed_or_error = make_matrix(length);
	board->filled_cells = 0;
	board->num_errors = 0;
	board->isInit = TRUE;
}

void update_err_board(sudoku_board *board, STATUS s)
{
	/*
	 * Updates the erroneous markings over all of the board.
	 * Assumes board is initialized.
	 */
	int i,j,length,hor,ver;

	length = board->length;
	hor = board->block_horizontal;
	ver = board->block_vertical;

	for(j = 1; j <= length; j++)
	{
		update_err_row(board, j, s); /*updates all rows*/
	}

	for(i = 1; i <= length; i++)
	{
		update_err_column(board, i, s); /*updates all columns*/
	}

	for(j = 1; j <= length; j+=ver) /*we only want the first entry of each block*/
	{
		for(i = 1; i <= length; i+=hor)  /*we only want the first entry of each block*/
		{
			update_err_block(board, i, j, s); /*updates all blocks*/
		}
	}
}

void load_puzzle_from_file(sudoku_board *board, FILE *file, STATUS s)
{
	/*
	 * Loads file into board. If there is data in board, it is replaced.
	 * Frees matrices of board if their sizes don't match with the new sizes, and allocates new matrices.
	 * Uses exit() if encountered a memory allocation error and prints the error message.
	 * Assumes file is an already opened stream on reading mode. Does not close the stream.
	 * For practical use, this method will not accept a puzzle with horizontal/vertical block length
	 * above 5. In this case, a message will be printed and the playing board will stay the same.
	 */
	int MAX_SIDE_LENGTH = 5;
	int hor,ver,length,i=1,j=1;
	char str[1024];
	char *delim = " \t\r\n";
	char c;
	int k, val;
	/*for saving previous values for reverting in case of error*/
	int **prev_cells = NULL, **prev_solution = NULL, **prev_fixed_or_error = NULL;
	int prev_length = 0, prev_hor = 0, prev_ver = 0;
	int prev_filled_cells = 0, prev_num_errors = 0, prev_isInit = board->isInit;
	/*variables for the moves list*/
	doublyLinkedList prev_move_list_head = NULL;
	doublyLinkedNode *prev_recent_move = NULL;

	if(fscanf(file,"%d %d",&ver,&hor)<2)/*reads block size*/
	{
		printf("Error: File is effectively empty. Load failed.\n");
		return;
	}
	if(hor<1|| ver<1)
	{
		printf("Error: Given board size is illegal. Load failed.\n");
		return;
	}
	if(hor>MAX_SIDE_LENGTH || ver>MAX_SIDE_LENGTH)
	{
		printf("Error: Given board size is not supported. The horizontal and vertical length of blocks\n"
				"is limited to be %d or lower. Load failed.\n", MAX_SIDE_LENGTH);
		return;
	}
	length = hor * ver;
	if(prev_isInit)
	{
		/*saving previous values for reverting in case of error*/
		prev_length = board->length;
		prev_hor = board->block_horizontal;
		prev_ver = board->block_vertical;
		prev_filled_cells = board->filled_cells;
		prev_num_errors = board->num_errors;

		prev_cells = make_matrix(prev_length);
		copy_matrix(board->cells, prev_cells,prev_length);

		prev_solution = make_matrix(prev_length);
		copy_matrix(board->solution, prev_solution,prev_length);

		prev_fixed_or_error = make_matrix(prev_length);
		copy_matrix(board->fixed_or_error, prev_fixed_or_error,prev_length);

		/*The previous board is initialized so there is a move list*/
		prev_move_list_head = move_list_head;
		prev_recent_move = recent_move;

	}
	if(!prev_isInit || prev_length!=hor*ver)
	{
		init_board(board,hor,ver); /*making sure that board is initialized correctly*/
	}
	else
	{
		board->filled_cells = 0;
		board->num_errors = 0;
	}

	/*doesn't matter what was the stage of the previous board, we need a new move list*/
	initialize_move_list();

	while(fscanf(file,"%s",str)>0)
	{
		if(j>length)
		{
			printf("Warning: File contains unused information. Loaded board may be incorrect.\n");
			break;
		}
		for(k = 1; (c=str[k])!='\0'; k++)/*looking to see if cell is fixed*/
		{
			if(c=='.')
			{
				board->fixed_or_error[i-1][j-1] = 1; /*fix cell*/
				str[k] = '\0'; /*removing the '.', since no digits are supposed to be afterwards, we can end the string here*/
				if(!strchr(delim,str[k+1])) /*the string didn't end where it's supposed to, wrong formatting*/
				{
					printf("Error: Cells  at (%d,%d) are not legally separated in file. Reverting to previous board.\n",i,j);
					goto REVERT;
				}
				break;
			}
		}
		if(c!='.')/*if cell isn't fixed make sure it isn't marked as fixed or error due to previous board*/
		{
			board->fixed_or_error[i-1][j-1] = 0;
		}
		val = atoi(str);
		if( (val<0) || (val>length) ) /*digit is illegal or fixed empty cell*/
		{
			printf("Error: Cell (%d,%d) contains illegal value. Reverting to previous board.\n",i,j);
			goto REVERT;
		}
		else if (val==0 && c=='.')
		{
			printf("Error: Cell (%d,%d) is both empty and fixed. Reverting to previous board.\n",i,j);
			goto REVERT;
		}
		board->cells[i-1][j-1] = val;
		if(val!=0)
		{
			board->filled_cells++;
		}

		/*go to next cell*/
		i = i%length+1;
		j = (i==1)?j+1:j;
	}

	if(j<=length)/*EOF reached but board was not filled*/
	{
		printf("Error: EOF reached and only %d out of %d cells were filled. Reverting to previous board.\n",(j-1)*length+i-1,length*length);
		goto REVERT;
	}
	/*if we did not revert*/
	/*free the previous fields because we don't need them*/
	free_matrix(prev_cells, prev_length);
	free_matrix(prev_solution, prev_length);
	free_matrix(prev_fixed_or_error, prev_length);
	free_move_list(prev_move_list_head);

	update_err_board(board,s);
	print_board(board,s);
	return;

	REVERT:
	/*no matter what was the state of the prev board, we made a new move list*/
	free_move_list(move_list_head);
	/*freeing the new allocated fields that we throw away*/
	free_matrix(board->cells,length);
	free_matrix(board->solution,length);
	free_matrix(board->fixed_or_error,length);

	/*re-assignment of the old values*/
	board->cells = prev_cells;
	board->solution = prev_solution;
	board->fixed_or_error = prev_fixed_or_error;
	board->length = prev_length;
	board->block_horizontal = prev_hor;
	board->block_vertical = prev_ver;
	board->num_errors = prev_num_errors;
	board->filled_cells = prev_filled_cells;
	board->isInit = prev_isInit;
	move_list_head = prev_move_list_head;
	recent_move = prev_recent_move;
}

void edit_no_param(sudoku_board *board)
{
	/*
	 * Changes data in board to a 9 by 9 empty sudoku board.
	 * Frees matrices of board if their sizes don't match a 9x9 normal board, and allocates new matrices.
	 * If encountered a memory allocation error, prints the error message and terminates the program.
	 * Otherwise, prints the board to the user.
	 */
	const int block_size = 3;
	if(board==NULL)
	{
		/*printf("debug (edit_no_param): board is NULL!!\n");debug*/
		return;
	}
	init_board(board,block_size,block_size);
	free_move_list(move_list_head);
	/*printf("Finished freeing all the move list.\n");debug*/
	initialize_move_list();
	print_board(board,EDIT);
	return;
}

BOOL autofill(sudoku_board *board, STATUS s)
{
	/*
	 * Automatically fills cells which contained a single legal value before this command was executed.
	 * Assumes the mode is Solve and the board is not erroneous.
	 * The method prints the board if the command was executed successfully.
	 * Some cells may be filled with values that are erroneous (two neighbors with the same value).
	 * Returns whether the last cell of the board was filled.
	 */
	LINKEDLIST i_vals,j_vals,digits, legal_vals, posi,posj,posdig;
	int i,j,length;
	BOOL board_has_changed = FALSE;/*change in the board means there was an empty cell that was filled with a non zero value*/

	/*doublyLinkedList tmp;*/ /*that's a surprise tool that will help us later*/

	append_new_move("autofill");

	i_vals = j_vals = digits = NULL;

	length = board->length;

	/*go over all cells in the board*/
	for(i = 1; i <= length; i++)
	{
		for(j = 1; j <= length; j++)
		{
			if(get(board,i,j) == 0)
			{
				/*found an empty cell that can be filled*/
				legal_vals = get_legal_values(board,i,j,FALSE);
				if(getLength(legal_vals)==1) /*only one legal value, exactly what we need for autofill*/
				{
					/*save the location of that cell and the legal value we want to fill*/
					appendToLinkedList(&i_vals,i);
					appendToLinkedList(&j_vals,j);
					appendToLinkedList(&digits,legal_vals->data); /*the data of the first and only element*/

					destroyLinkedList(legal_vals); /*free the now unused legal values list*/
				}
			}
		}
	}

	/*initializing the "index variables" to go over our lists*/
	posi = i_vals;
	posj = j_vals;
	posdig = digits;

	while(NULL != posi)
	{
		append_ijval_to_recent_move(posi->data, posj->data, 0, posdig->data); /*saves a change to the board*/

		board->cells[posi->data-1][posj->data-1] = posdig->data; /*places the only possible value of the cell on that cell*/
		board->filled_cells++;

		/*"i++ except it's three lists*/
		posi = posi->next;
		posj = posj->next;
		posdig = posdig->next;
	}

	/*we don't need those lists anymore*/
	destroyLinkedList(i_vals);
	destroyLinkedList(j_vals);
	destroyLinkedList(digits);

	board_has_changed = (((move*)recent_move->data)->set_list_end)!=NULL;
	if(board_has_changed)
	{
		update_err_board(board,s);
	}
	else
	{
		print_no_change_in_autofill();
	}

	print_board(board,SOLVE);

	return board->filled_cells==length*length;
}

static void empty_cells(sudoku_board *board)
{
	/*
	 * Assigns 0 to all the cells in board->cells.
	 * Assumes board is initialized.
	 */
	int length = board->length;
	int **cells = board->cells;
	int i,j;
	for(i=0; i<length ;i++)
	{
		for(j=0; j<length ;j++)
		{
			cells[i][j]=0;
		}
	}
}

void reset(sudoku_board *board, STATUS s)
{
	/*
	 * Undo all moves, reverting the board to its original state.
	 * Assumes the mode is Edit or Solve (meaning board and the move list are initialized).
	 * The move list in not cleared, the pointer is just moved to the head.
	 * The method prints the board when done.
	 */
	if(recent_move==NULL)
	{/*We are right after a load move*/
		print_no_change_in_reset();
		return;
	}
	else
	{/*We are right after a command that the user did*/
		empty_cells(board);/*makes board->cells entirely out of zeros*/
		restore_prev_values_of_move(board, move_list_head,s);
		recent_move = NULL; /*because now we are right after a load move*/
		print_board(board, s);
		/*print_reset_done();*/
		return;
	}
}
