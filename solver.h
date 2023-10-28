#ifndef __SOLVER_H__
#define __SOLVER_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main_aux.h"
#include "definitions.h"
#include "matrix.h"
#include "gurobi_c.h"
#include "msg_prints.h"
#include "linked_list.h"
#include "game.h"


/*
 * This module solves the Sudoku board using exhaustive backtracking, LP or ILP.
 */

/*
 * solves the board using ILP and stores the solution in board->solution.
 * assumes board is initialized.
 * returns TRUE/FALSE if the board is solvable/unsolvable or ERROR if such encountered.
 */
BOOL solve_board(sudoku_board *board);

/*
 * Gets an empty sudoku board and randomly generates a full board. Stores the result in board.solution.
 * Returns: TRUE if a solution found, FALSE if no solution exists.
 *
 */
BOOL generate_full_board(sudoku_board *board);

/*
 * Given an initialized sudoku board, counts and returns the amount of possible solutions for the board using exhaustive backtracking.
 */
int exhaustive_backtracking(sudoku_board *board);

/*
 * gets the index of a 3d cubic matrix and converts it to a flattened version index.
 * assumes i, j, k are legal given length, and that length > 0.
 * returns the converted index.
 * i,j,k >=1, return_val>=0
 */
int get_3d_index(int i, int j, int k, int length);

/*
 * Returns a flattened 3d matrix containing the scores of each digit in each cell given by the LP solver on success.
 * If an error occurred or the board was strongly unsolvable, returns NULL instead.
 * To get the ijk element, please use get_3d_index(i,j,k) to get the appropriate index.
 * If an element of the array is negative, it means this variable represented an illegal value and did not have a variable.
 * This function uses malloc, remember to free its return value content afterward.
 */
double* get_LP_scores(sudoku_board *board);

/*
 * Returns TRUE if Gurobi encountered a problem in the recent LP or ILP execution.
 * FALSE otherwise.
 * Useful when the function that uses Gurobi doesn't return ERROR on error.
 */
BOOL get_error_status();
#endif /*__SOLVER_H__*/
