#include "solver.h"

/*
 * This module solves the Sudoku board using exhaustive backtracking, LP or ILP.
 */

/*
 * A constant that says if gurobi encountered an error during the recent LP or ILP.
 * Useful when the function that uses Gurobi doesn't return ERROR on error.
 */
BOOL GRBERROR = FALSE;

/*
 * An array representing a 3D matrix. Cell ijk contains the index 'ind'
 * of the gurobi variables vector in which the corresponding gurobi variable is found.
 * Meaning gurobi_vars[ind] is 1 if the cell ij contains k. 0 otherwise.
 */
int* ijk_to_gurobi;

/*
 * A linked list of ijk values ordered according to their placement in the gurobi vector.
 */
LINKEDLIST gurobi_to_ijk = NULL;

LINKEDLIST get_possible_values(sudoku_board *board, int i, int j,
		BOOL is_deterministic) {
	/*
	 * returns a list of the numbers 1...length if is_deterministic, sorted array of legal values otherwise
	 * the first value in the array will be the number of elements
	 */
	int length = board->length;
	/*int *values = malloc((length+1)*sizeof(int));*/
	LINKEDLIST values = NULL;
	int k;
	/*int index; debug*/

	if (is_deterministic) {
		for (k = 1; k <= length; k++) {
			/*printf("appending %d\n",k); *//*DEBUG*/
			appendToLinkedList(&values, k); /*for the deterministic version, we use all possible values*/
		}
	} else {
		/*printf("entering legal values\n"); *//*DEBUG*/
		values = get_legal_values(board, i, j, TRUE);
	}
	return values;
}

int next_cell_i(int i, int length) {
	/*
	 * calculates the column index of the next cell in the backtracking algorithm.
	 * returns i+1 if i<length, or 1 if i == length, assuming 1<=i<=length
	 */
	return (i % length) + 1;
}

int next_cell_j(int i, int j, int length) {
	/*
	 * calculates the row index of the next cell in the backtracking algorithm.
	 * returns j if i<length, or j+1 if i == length, assuming 1<=i<=length
	 */
	if (i == length) {
		return j + 1;
	}
	return j;
}

void remove_at_index(int *values, int k) {
	/*
	 * virtually removes the k-th value of values. example: values = [5,1,3,5,6,9], k = 3 then values become [4,1,3,6,9(,9)]. "5" was the k-th value, and t
	 */
	int i;
	int n = values[0];
	for (i = k; i < n; i++) {
		values[i] = values[i + 1];
	}
	values[0]--;
}

int exhaustive_backtracking(sudoku_board *board) {
	/*
	 * Given an initialized sudoku board, counts and returns the amount of possible solutions for the board using exhaustive backtracking.
	 */
	int length = board->length;
	int num_solutions = 0;
	int i, j, digit;
	doublyLinkedList stack = NULL;
	ijval *stack_node; /*the prev_val in ijval will represent the digit to be checked as a possible solution*/

	/*initializing the "recursion"*/
	stack_node = new_ijval(1, 1, 1, 0);/*the cell (1,1) is inserted the first legal value, then the rest of the board is checked*/
	push(&stack, stack_node);
	copy_matrix(board->cells, board->solution, length); /*making solution ready for solving*/

	while (stack != NULL) /*as long as the stack is not empty*/
	{
		stack_node = (ijval*) (stack->data); /*peek top of the stack ("What do I need to check?")*/
		digit = stack_node->prev_val;
		i = stack_node->i;
		j = stack_node->j;
		/*printf("debug: i = %d, j = %d\n", i, j);debug*/

		if (digit > length) {
			board->solution[i - 1][j - 1] = 0;
			free(pop(&stack)); /*remove exhausted option: "return" */
			continue;
		}

		/*"else"*/
		if (j > length) {
			/*we reached the end without inserting an illegal value, new solution has been found!*/
			num_solutions++;

			/*print_matrix(board->solution, length);debug*/
			/*printf("\n");debug*/

			/* "return" */
			free(pop(&stack));
			continue;
		}

		/*"else"*/

		if (board->cells[i - 1][j - 1] == 0) {
			if (is_legal(board, i, j, digit, TRUE)) {/*if this cell is empty on the playing board and the digit is a legal value*/
				board->solution[i - 1][j - 1] = digit; /*trying to put digit at the (i,j) place*/
				stack_node->prev_val++; /*we'll try the next digit next time*/
				/*beginning the same process on the next cell. "the recursive call"*/
				stack_node = new_ijval(next_cell_i(i, length),
						next_cell_j(i, j, length), 1, 0);
				push(&stack, stack_node);
				continue;/*go to check the head again*/
			} else {/*the cell is empty but the digit is not legal*/
				stack_node->prev_val++;/*check the next digit next time*/
				continue;/*go to check the head again*/
			}
		} else {/*if this cell contains a value on the playing board, we don't need to change it at all*/
			free(pop(&stack));/*destroy this cell at the stack so we won't come back to it*/
			/*begin the same process on the next cell. "the recursive call"*/
			stack_node = new_ijval(next_cell_i(i, length),
					next_cell_j(i, j, length), 1, 0);
			push(&stack, stack_node);
			continue;/*go to check the head again*/
		}
	}
	return num_solutions;
}

int get_3d_index(int i, int j, int k, int length)
{
	/*
	 * gets the index of a 3d cubic matrix and converts it to a flattened version index.
	 * assumes i, j, k are legal given length, and that length > 0.
	 * returns the converted index.
	 * i,j,k >=1, return_val>=0
	 */
	i--;
	j--;
	k--;
	return i + j*length + k*length*length;
}

int create_gurobi_conversion(sudoku_board *board)
{
	/*
	 * Goes over all of board's cells and finds the gurobi xijk variables that are legal.
	 * Inserts the data to the conversion "matrix" 'ijk_to_gurobi' and to the inverse
	 * conversion 'gurobi_to_ijk'.
	 *
	 * Returns the amount of gurobi variables found.
	 * If there is an empty cell with no legal values, all resources are freed and the function returns -1.
	 *
	 * Remember to free (and NULLify) ijk_to_gurobi and gurobi_to_ijk before using.
	 */
	int i,j,k;
	int length;
	int legal_var_amount = 0;
	int linear_index;

	BOOL legal_value_exists;

	/*LINKEDLIST pos;*/ /*DEBUG*/

	length = board->length;

	ijk_to_gurobi = malloc(length*length*length*sizeof(int)); /*initialize ijk_to_gurobi*/
	check_alloc(ijk_to_gurobi,"create_gurobi_conversion");

	/*goes over the cells of the board to find legal values*/
	for(j = 1; j <= length; j++)
	{
		for(i = 1; i <= length; i++)
		{
			if( get(board,i,j) == 0 ) /*empty cell, need to find legal values for the cell*/
			{
				legal_value_exists = FALSE;

				for(k = 1 ; k <= length; k++)
				{
					linear_index = get_3d_index(i,j,k,length); /*target ijk index*/

					if(is_legal(board,i,j,k,FALSE)) /*we need a varible, update conversion tables*/
					{
						ijk_to_gurobi[linear_index] = legal_var_amount;
						legal_var_amount++;
						legal_value_exists = TRUE;
						appendToLinkedList(&gurobi_to_ijk,linear_index);


					}
					else
					{
						ijk_to_gurobi[linear_index] = -1; /*illegal value, no need for variable.*/
					}
				}
				if(!legal_value_exists)
				{
					/*
					 * If there is an empty cell without a legal value, then there is no solution.
					 * Cells with no legal values will not have variables and no constraint will be violated,
					 * causing an unsolvable board to seem solvable to gurobi.
					 */
					free(ijk_to_gurobi);
					destroyLinkedList(gurobi_to_ijk);
					gurobi_to_ijk = NULL;
					return -1;
				}
			}
			else /*cell is filled, all values do not coresspond to any variable in gurobi*/
			{
				for(k = 1; k<=length; k++) /*goes over all the digits and makes them "illegal"*/
				{
					ijk_to_gurobi[get_3d_index(i,j,k,length)] = -1;
				}
			}
		}
	}

	/*ONE GIANT DEBUG*/
	/*i = 0;
	for(pos = gurobi_to_ijk; pos!=NULL;pos = pos->next )
	{
		j = ijk_to_gurobi[pos->data];
		printf("gurobi_index: %d; converted ijk index: %d; back again: %d\n", i,pos->data,j);
		if(i!=j)
		{
			printf("OH NO!!\n");
			break;
		}

		i++;
	}*/

	return legal_var_amount;
}

/*
 * makes a name for the constraint given by constraint_name.
 * assumes constraint_name is already allocated, and values inside of it will be overridden.
 * the name will be of the form: "<begin> <num> digit <digit>"
 * fill begin with "row", "column", "block", or anything else that makes sense.
 */
void make_constraint_name(char* begin, int num, int digit, char* constraint_name)
{
	char int_str_holder[16];

	strcpy(constraint_name,begin); /*constraint_name is now effectively empty*/


	sprintf(int_str_holder, " %d digit ", num);
	strcat(constraint_name,int_str_holder);


	sprintf(int_str_holder, "%d", digit);
	strcat(constraint_name,int_str_holder);

}

/*
 * converts the variable values gurobi has given to a flattened 3d matrix of size length*length*lentgh.
 * non existent variables get -1.
 * stores the values in target, which is assumed to be allocated.
 */
void gurobi_solution_to_array(const double* gurobi_sol, double* target, int length)
{
	int i,j,k;
	int gurobi_index,ijk_index;

	for(i = 1; i <= length; i++)
	{
		for(j = 1; j <= length; j++)
		{
			for(k = 1; k <= length; k++)
			{
				ijk_index = get_3d_index(i,j,k,length);
				gurobi_index = ijk_to_gurobi[ijk_index];

				if(gurobi_index < 0) /*no variable, set to minus 1*/
				{
					target[ijk_index] = -1.0;
				}
				else /*there is a variable, put its value*/
				{
					target[ijk_index] = gurobi_sol[gurobi_index];
				}

			}
		}
	}
}

BOOL generic_LP_solve(sudoku_board *board, BOOL isInt, double* sol)
{
	/*
	 * Uses Integer or non-Integer Linear Programming to solve the board, depends on isInt (TRUE is ILP).
	 * Stores the solution as a flattened 3d matrix inside sol, which is assumed to be allocated.
	 * returns TRUE if the board is solvable, FALSE if the board is unsolvable and ERROR if an error encountered.
	 */

	/*
	 * The model:
	 * binary variables x[i,j,v] indicate whether cell <i,j> contains value 'v'.
	 * i,j are the indexes of all the empty cells on the given board.
	 * 'v' is all the legal values of the cell <i,j> according to the given board.
	 * The constraints are as follows:
	 *  1. Each cell must take exactly one value (sum_v x[i,j,v] = 1)
	 *  2. Each legal value is used exactly once per row (sum_i x[i,j,v] = 1)
	 *  3. Each legal value is used exactly once per column (sum_j x[i,j,v] = 1)
	 *  4. Each legal value is used exactly once per block (sum_block x[i,j,v] = 1)
	 */
	/*constraint names*/
	char constraint_names[32];

	int length = board->length;
	int n; /*number of actual variables*/
	int hor = board->block_horizontal;
	int ver = board->block_vertical;

	GRBenv *env = NULL;
	GRBmodel *model = NULL;

	int error = 0;

	double *gurobi_sol; /*array for the solution. needs to be dynamically allocated because we don't know how many variables there are*/
	int *ind; /*array of variable indices at constraints. needs to be dynamically allocated because we don't know how many variables there are*/
	double *coef;/*array of variable coefficients at constraints. malloc blabla...*/

	double *obj = NULL; /*array containing the coefficients of all the variables in the objective function*/

	char *vtype; /*an array containing the variable types*/

	int optimstatus = -42;

	double *lb = NULL, *ub = NULL; /*lower bound and upper bound*/

	char binary_or_continuous = isInt ? GRB_BINARY:GRB_CONTINUOUS;

	/*loop indices*/
	int i,j,k,I,J,count;

	int gurobi_index; /*for storing varables index*/

	GRBERROR = FALSE;

	n = create_gurobi_conversion(board); /*get the variables we need and their number*/

	if(n == -1) /* i.e. there exists an empty cell with no legal value*/
	{
		return FALSE;
	}

	vtype = malloc(sizeof(char)*n); /*allocate vtype before model creation*/

	check_alloc(vtype,"generic_LP_solve");

	/*set up variables' type*/
	for(i = 0; i < n; i++)
	{
		vtype[i] = binary_or_continuous;
	}

	if(!isInt) /*range constraints for the variables in the continuous case*/
	{
		ub = malloc(sizeof(double)*n);
		lb = malloc(sizeof(double)*n);
		check_alloc(ub,"generic_LP_solve");
		check_alloc(lb,"generic_LP_solve");
		for(i = 0; i < n; i++)
		{
			ub[i] = 1;
			lb[i] = 0;
		}
	}

	/* Create environment - log file is sudoku_model.log */
	error = GRBloadenv(&env, "sudoku_model.log");
	if (error)
	{
		printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));
		GRBERROR = TRUE;
		return ERROR;
	}

	error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
	if (error)
	{
		printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));
		GRBERROR = TRUE;
		return ERROR;
	}

	/* Create an empty model named "sudoku_model" */
	error = GRBnewmodel(env, &model, "sudoku_model", n, obj, lb, ub, vtype, NULL);
	if (error)
	{
		printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));
		GRBERROR = TRUE;
		return ERROR;
	}

	/*sudoku constraints*/
	coef = malloc(sizeof(double)*length); /*there are length variables in each sudoku constraint*/
	ind = malloc(sizeof(int)*length); /*there are length variables in each sudoku constraint*/
	check_alloc(coef,"generic_LP_solve");
	check_alloc(ind,"generic_LP_solve");
	for(i = 0; i < length; i++)
	{
		coef[i] = 1.0; /*all variables participating get 1 => sum over variables (for all the constraints)*/
	}
	for(k = 1; k <= length; k++) /*each digit gets a constraint*/
	{
		/*column constraints*/
		for(i = 1; i <= length; i++)
		{
			count = 0;
			for(j = 1; j <= length; j++)
			{
				gurobi_index = ijk_to_gurobi[get_3d_index(i,j,k,length)];
				if(gurobi_index>=0)
				{
					ind[count] = gurobi_index; /*the count variable in the constraint corresponds to the gurobi_index*/
					count++;
				}
			}
			if(count>0)
			{
				make_constraint_name("column",i,k,constraint_names);
				error = GRBaddconstr(model, count, ind, coef, GRB_EQUAL, 1.0, constraint_names);
			}
			if(error) goto END;
		}

		/*block constraints*/
		for(J = 0; J < hor; J++)
		{
			for(I = 0; I < ver; I++)
			{
				/*this is executed for each block*/
				count = 0;

				for(i = hor*I+1; i<=hor*(I+1); i++)
				{
					for(j = ver*J+1; j<=ver*(J+1); j++)
					{
						gurobi_index = ijk_to_gurobi[get_3d_index(i,j,k,length)];

						if(gurobi_index >= 0)
						{
							ind[count] = gurobi_index; /*the count variable in the constraint corresponds to the gurobi_index*/
							count++;
						}
					}
				}
				if(count>0)
				{
					make_constraint_name("block", I+J*ver, k, constraint_names);
					error = GRBaddconstr(model, count, ind, coef, GRB_EQUAL, 1.0, constraint_names);
					if(error) goto END;
				}
			}
		}

		/*row constraints*/
		for(j = 1; j <= length; j++)
		{
			count = 0;
			for(i = 1; i <= length; i++)
			{
				gurobi_index = ijk_to_gurobi[get_3d_index(i,j,k,length)];

				if(gurobi_index>=0)
				{
					ind[count] = gurobi_index; /*the count variable in the constraint corresponds to the gurobi_index*/
					count++;
				}
			}
			if(count>0)
			{
				make_constraint_name("row",j,k,constraint_names);
				error = GRBaddconstr(model, count, ind, coef, GRB_EQUAL, 1.0, constraint_names);
				if(error) goto END;
			}
		}
	}

	/*"there is only one value in each cell" constraints*/
	for(i = 1; i <= length; i++)
	{
		for(j = 1; j <= length; j++)
		{
			count = 0;
			for(k = 1; k <= length; k++)
			{
				gurobi_index = ijk_to_gurobi[get_3d_index(i,j,k,length)];
				if(gurobi_index>=0)
				{
					ind[count] = gurobi_index; /*the count variable in the constraint corresponds to the gurobi_index*/
					count++;
				}
			}
			if(count>0)
			{
				make_constraint_name("values_pre_cell",i+length*j,k,constraint_names);
				error = GRBaddconstr(model, count, ind, coef, GRB_EQUAL, 1.0, constraint_names);
				if(error) goto END;
			}
		}


	}

	error = GRBoptimize(model); /*the actual optimization happens here!*/
	if (error) goto END;
	error = GRBgetintattr(model, "status", &optimstatus);
	if (error) goto END;

	END:

	/*freeing memory:*/
	free(vtype);
	free(coef);
	free(ind);
	free(lb);
	free(ub);
	destroyLinkedList(gurobi_to_ijk);
	gurobi_to_ijk = NULL;

	if(error)
	{
		GRBfreemodel(model);
		GRBfreeenv(env);
		GRBERROR = TRUE;
		return ERROR;
	}

	if(optimstatus == GRB_OPTIMAL) /*GUROBI found a solution*/
	{
		gurobi_sol = malloc(sizeof(double)*n);
		check_alloc(gurobi_sol,"generic_LP_solve");

		error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, n, gurobi_sol);
		gurobi_solution_to_array(gurobi_sol,sol,length);

		free(gurobi_sol);
		free(ijk_to_gurobi);
		GRBfreemodel(model);
		GRBfreeenv(env);

		if(error)
		{
			GRBERROR = TRUE;
			return ERROR;
		}
		return TRUE;
	} /*no solution found, board is unsolvable*/
	else
	{
		free(ijk_to_gurobi);
		GRBfreemodel(model);
		GRBfreeenv(env);

		if(optimstatus == GRB_INFEASIBLE) /*model is infeasible, meaning board is unsolvable*/
		{
			return FALSE;
		}
		else /*optimization has stopped early*/
		{
			GRBERROR = ERROR;
			return ERROR;
		}
	}

}

BOOL solve_board(sudoku_board *board)
{
	/*
	 * solves the board using ILP and stores the solution in board->solution.
	 * assumes board is initialized.
	 * returns TRUE/FALSE if the board is solvable/unsolvable or ERROR if such encountered.
	 */
	BOOL isSolvable;

	int length = board->length;
	int i,j,k,index;
	double score;

	double* sol = malloc(sizeof(double)*length*length*length);
	check_alloc(sol,"solve");
	GRBERROR = FALSE;
	isSolvable = generic_LP_solve(board,TRUE,sol);

	copy_matrix(board->cells,board->solution,length);

	if(isSolvable==ERROR)
	{
		GRBERROR = TRUE;
	}
	if(isSolvable==TRUE)
	{
		for(j = 1; j <= length; j++)
		{
			for(i = 1; i <= length; i++)
			{
				for(k = 1; k <= length; k++)
				{
					index = get_3d_index(i,j,k,length);
					score = sol[index];

					if(score==1.0)
					{
						board->solution[i-1][j-1] = k;
						break;
					}
				}
			}
		}
	}

	free(sol);
	return isSolvable;
}

double* get_LP_scores(sudoku_board *board)
{
	/*
	 * Returns a flattened 3d matrix containing the scores of each digit in each cell given by the LP solver on success.
	 * If an error occurred or the board was strongly unsolvable, returns NULL instead.
	 * Use get_error_status() to see if there was an error.
	 * To get the ijk element, please use get_3d_index(i,j,k) to get the appropriate index.
	 * If an element of the array is negative, it means this variable represented an illegal value and did not have a variable.
	 * This function uses malloc, remember to free its return value content afterward.
	 */
	int length = board->length;
	BOOL LP_status;
	double* sol = malloc(sizeof(double)*length*length*length);
	check_alloc(sol,"get_LP_scores");
	GRBERROR = FALSE;
	LP_status = generic_LP_solve(board,FALSE,sol);
	if(LP_status!=TRUE)
	{/*board is unsolvable or error*/
		free(sol);
		if(LP_status==ERROR)
		{
			GRBERROR = TRUE;
		}
		return NULL;
	}
	/*board is solvable*/
	return sol;
}


BOOL get_error_status()
{
	/*
	 * Returns TRUE if Gurobi encountered a problem in the recent LP or ILP execution.
	 * FALSE otherwise.
	 * Useful when the function that uses Gurobi doesn't return ERROR on error.
	 */
	return(GRBERROR);
}
