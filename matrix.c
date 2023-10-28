#include "matrix.h"

/*
 * This module creates and handles 2D square matrices.
 * Implemented with 2D arrays.
 */

int** make_matrix(int length)
{
	/*returns a pointer to square matrix of length "length", initialized to 0*/
	/*USES MALLOCS, use free_matrix to free*/
	int i;

	int **M = (int**)(malloc(sizeof(int*)*length));
	check_alloc(M,"make_matrix");

	for(i = 0; i < length; i++)
	{
		M[i] = (int*)(calloc(length,sizeof(int)));
		check_alloc(M[i],"make_matrix");
	}
	return M;
}

void free_matrix(int** M, int length)
{
	/*frees up allocated space of a square matrix M of length "length"*/

	int i;
	if(M!=NULL)
	{
		for(i = 0; i < length; i++)
		{
			free(M[i]);
		}

		free(M);
	}
}

int get_matrix(int **M,int i,int j,int length)
{
	/*
	 * returns the i,j value of a matrix (column number, row number)
	 * if i,j are out of bounds (not between 1 and length) or board is NULL, returns ERROR.
	 */
	if((M==NULL) || (i<=0) || (j<=0) || (i>length) || (j>length))
	{
		return ERROR;
	}
	else
	{
		return M[i-1][j-1];
	}
}

void copy_matrix(int **M_from,int **M_to,int length)
{
	/*
	 * copies the value of square matrix M_from to M_to, assuming they are both of size length.
	 * if one of these is null, it prints and does nothing else.
	 */
	int i,j;
	if(M_from == NULL)
	{
		printf("M_from is null\n");
		return;
	}
	if(M_to == NULL)
	{
		printf("M_to is null\n");
		return;
	}
	for (i = 0; i < length; i++)
	{
		for (j = 0; j < length; j++)
		{
			M_to[i][j] = M_from[i][j];
		}
	}
}

void print_matrix(int** M,int length)
{
	/*
	 * prints a square matrix M of size length without delimiters aside for a single space between columns and \n between rows.
	 * treats M[i] as the columns of the matrix
	 */
	int i,j;
	for (j = 0; j < length; j++)
	{
		for (i = 0; i < length; i++)
		{
			printf("%d ",M[i][j]);
		}
		printf("\n");
	}
}
