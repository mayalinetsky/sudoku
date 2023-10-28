#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "msg_prints.h"
#include "definitions.h"
#include "matrix.h"
#include "move_list.h"

/**
 * This module contains functions that help the other modules in the Sudoku program,
 * but they do no belong to any of them.
 */

/*
 * makes all calls necessary to end game. Use before terminating.
 * (This is here because a game is not responsible for closing itself :D)
 */
void exit_game(sudoku_board *board);

/*
 * A VERY USEFUL METHOD which prints a memory allocation error
 * and TERMINATES THE PROGRAM if the pointer 'thing' is NULL.
 * Have fun allocating.
 */
void check_alloc(void* thing, char* func_name);

/*
 * returns a random index between 1 and n.
 * does not call rand() if n==1.
 */
int get_index(int n);

/* Returns a random number from arr[] according to
 * distribution array defined by freq[]. n is size of arrays.
 * Taken from: https://www.geeksforgeeks.org/random-number-generator-in-arbitrary-probability-distribution-fashion/
 */
int myRand(int arr[], int freq[], int n);

/*
 * Returns the minimum value inside the array 'arr' with length 'length'.
 * Assumes arr is not NULL.
 */
double get_min(double arr[], int length);
#endif /*MAIN_AUX_H_*/
