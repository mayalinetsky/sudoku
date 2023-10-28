#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdlib.h>
#include <stdio.h>
#include "definitions.h"
#include "main_aux.h"

/*
 * This module creates and handles 2D square matrices.
 */

/*returns a pointer to square matrix of length "length", initialized to 0*/
/*USES MALLOCS, use free_matrix to free*/
int** make_matrix(int length);

/*frees up allocated space of a square matrix M of length "length"*/
void free_matrix(int** M, int length);

/*
 * returns the i,j value of a matrix (column number, row number)
 * if i,j are out of bounds (not between 1 and length) or board is NULL, returns ERROR.
 */
int get_matrix(int **M,int i,int j,int length);

/*
 * prints a square matrix M of size length without delimiters aside for a single space between columns and \n between rows.
 * treats M[i] as the columns of the matrix
 */
void print_matrix(int** M,int length);

/*
 * copies the value of square matrix M_from to M_to, assuming they are both of size length.
 * if one of these is null, it prints and does nothing else.
 */
void copy_matrix(int **M_from,int **M_to,int length);


#endif /* MATRIX_H_ */
