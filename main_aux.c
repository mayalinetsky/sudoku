#include "main_aux.h"

void check_alloc(void* thing, char* func_name)
{
	/*
	 * A VERY USEFUL METHOD which prints a memory allocation error
	 * and TERMINATES THE PROGRAM if the pointer 'thing' is NULL.
	 * Have fun allocating.
	 */
	if(thing==NULL)
	{
		print_memory_allocation_error_msg(func_name);
		exit(MEM_ALLOC_ERR);
	}
}

static BOOL free_board(sudoku_board *board)
{
	/*
	 * Frees up space allocated by "initialize".
	 * Returns TRUE if succeeded, FALSE otherwise.
	 */
	int length;

	if(board == NULL)
	{
		return FALSE;
	}
	if(board->isInit)
	{
		length = board->length;
		/*frees up allocated memory of variables*/
		free_matrix(board->cells,length);
		free_matrix(board->fixed_or_error,length);
		free_matrix(board->solution,length);
	}
	/*frees up the memory allocated for the board itself*/
	free(board);

	return TRUE;
}

void exit_game(sudoku_board *board)
{
	/*
	 * makes all calls necessary to end game. Use before terminating.
	 * (This is here because a game is not responsible for closing itself :D)
	 */
	free_move_list(move_list_head);
	free_board(board);
	printf("Exiting...\n");
}

int get_index(int n) {
	/*
	 * returns a random index between 1 and n.
	 * does not call rand() if n==1.
	 */
	if (n == 1) {
		return 1;
	}
	return (rand() % n) + 1;
}

static int findCeil(int arr[], int r, int l, int h)
{
	/*
	 * Utility function for myRand to find ceiling of r in arr[l..h].
	 * Taken from: https://www.geeksforgeeks.org/random-number-generator-in-arbitrary-probability-distribution-fashion/
	 */
	int mid;
	while (l < h)
	{
		mid = l + ((h - l) >> 1); /* Same as mid = (l+h)/2*/
		(r > arr[mid]) ? (l = mid + 1) : (h = mid);
	}
	return (arr[l] >= r) ? l : -1;
}

int myRand(int arr[], int freq[], int n)
{
	/* The main function that returns a random number from arr[] according to
	 * distribution array defined by freq[]. n is size of arrays.
	 * Taken from: https://www.geeksforgeeks.org/random-number-generator-in-arbitrary-probability-distribution-fashion/
	 */
	int *prefix, i, r, indexc;
	prefix = (int*)calloc(n, sizeof(int));
	check_alloc(prefix, "myRand");
	prefix[0] = freq[0];
	for (i = 1; i < n; ++i)
	{
		prefix[i] = prefix[i - 1] + freq[i];
	}

	/* prefix[n-1] is sum of all frequencies. Generate a random number*/
	/* with value from 1 to this sum*/
	r = (rand() % prefix[n - 1]) + 1;

	/* Find index of ceiling of r in prefix array*/
	indexc = findCeil(prefix, r, 0, n - 1);
	free(prefix);
	return arr[indexc];
}


double get_min(double arr[], int length)
{
	/*
	 * Returns the minimum value inside the array 'arr' with length 'length'.
	 * Assumes arr is not NULL.
	 */
	int i;
	double min = DBL_MAX;
	for(i=0; i<length; i++)
	{
		if(arr[i]<min)
		{
			min = arr[i];
		}
	}
	return min;
}
