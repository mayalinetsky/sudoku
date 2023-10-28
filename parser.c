#include "parser.h"

/*
 * This module gets commands from the user and executes them.
 * Prints detailed error messages when it is needed.
 */

static BOOL isWhole(float d)
{
	return d==(float)((int)d);
}

static int charToDig(char c)
{
	/*
	 * Converts the character c to a digit.
	 * Returns -1 on error (c is not an integer from 0 to 9)
	 */
	if(c<='9' &&c>='0')
	{
		return c-'0';
	}
	return -1;
}

static BOOL strToNum(char* str, float* d)
{
	/*
	 * Converts a string to a float.
	 * On success returns TRUE and d will point to the converted number.
	 * On failure (not a number) returns FALSE and d will stay the same;
	 */
	float num = 0;/*the converted number*/
	int dig; /*a digit from the string*/
	int cnt = 0; /*how many numbers are located to the right of the dot*/
	char *pos = str; /*position in str*/
	BOOL neg = FALSE; /*is the number negative?*/
	BOOL frac = FALSE;/*are we past a '.'?*/
	if(*pos == '-')/*checks for a negative number*/
	{
		neg = TRUE;
		pos++;
	}

	/*go over str*/
	for(;*pos != '\0'; pos++)
	{
		dig = charToDig(*pos);
		if(dig==-1)
		{/*the char wasn't a digit 0 to 9*/
			if(*pos == '.')
			{
				if(frac)
				{/*a dot in the fraction part? Outrageous!*/
					return FALSE;
				}
				/*oh it's a dot that starts the fraction part. We cool.*/
				frac = TRUE;
				continue;
			}
			/*the character is not a digit or a dot: not a number*/
			return FALSE;
		}

		/*add everything to num like it's an integer*/
		num *= 10;
		num += dig;

		if(frac){cnt++;}
	}

	/*now divide by 10 as many times as there are numbers right to the dot*/
	for( ; cnt>0; cnt--)
	{
		num/=10;
	}

	if(neg){num=-num;}
	*d = num;
	return TRUE;
}

static BOOL check_cell_c(char* x, float* X, char* y, float* Y, int length)
{
	/*
	 * Checks if (x,y) are valid coordinates for a cell: (column, row).
	 * Returns TRUE if valid and puts the converted values in X and Y as floats.
	 * Returns FALSE if invalid and doesn't touch X, Y.
	 * Prints error messages.
	 */
	char tmpstr[30]; /*temporary string. 27 characters + 3 characters for the board's length in base 10*/
	BOOL res; /*conversion result*/

	sprintf(tmpstr, "between 1 and %d, including.", length);

	res = strToNum(x, X);
	if(res==FALSE){print_nan("first ");return FALSE;}
	if(isWhole(*X)==FALSE){print_ni("first "); return FALSE;}
	if((*X)<1 || (*X)>length)
	{
		print_parameter_notInRange("first ", tmpstr);
		return FALSE;
	}

	res = strToNum(y, Y);
	if(res==FALSE){print_nan("second ");return FALSE;}
	if(isWhole(*Y)==FALSE){print_ni("second "); return FALSE;}
	if((*Y)<1 || (*Y)>length)
	{
		print_parameter_notInRange("second ", tmpstr);
	}

	return TRUE;
}

static BOOL check_cell_v(char* z, float* Z, int length)
{
	/*
	 * Checks if z is a valid value for a cell (integer between 0 and length).
	 * Returns TRUE if valid and puts the converted value in Z as a float.
	 * Returns FALSE if invalid and doesn't touch Z.
	 * Prints error messages.
	 */
	char tmpstr[30];

	if(strToNum(z, Z)==FALSE){print_nan("third ");return(FALSE);}
	if(isWhole(*Z)==FALSE){print_ni("third "); return(FALSE);}
	if((*Z)<0 || (*Z)>length)
	{
		sprintf(tmpstr, "between 0 and %d, including", length);
		print_parameter_notInRange("third ", tmpstr);
		return(FALSE);
	}
	return TRUE;
}

static BOOL continue_checking_solve(char *delimiters, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * If the command is invalid or there was a problem with the file, prints the matching error message.
	 * Else, executes load_puzzle_from_file
	 *(can encounter a mem aloc problem inside, in which case the program is terminated).
	 * Returns TRUE if the loaded puzzle is completely solved (all cells are filled and there are no errors),
	 * 		   FALSE if the loaded puzzle in not completely solved (can be with errors)
	 * 		   ERROR if the command is invalid (incorrect number of parameters/invalid file format/fopen error)
	 * 		   		 meaning the mode will not be switched.
	 */
	char *path, *p; /*inputs. p for extra invalid parameters*/
	FILE *f;
	int filled_cells, length, num_errors;

	path = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(0, "s");
		return(ERROR);
	}
	if(path==NULL)
	{
		print_not_enough_parameters(1, "");
		return(ERROR);
	}
	if(strstr(path, file_format)==NULL)
	{
		print_invalid_format();
		return(ERROR);
	}
	f = fopen(path, "r");
	if(f==NULL)
	{
		print_fopen_error();
		return(ERROR);
	}
	load_puzzle_from_file(board, f, SOLVE);
	fclose(f);
	length = board->length;
	filled_cells = board->filled_cells;
	num_errors = board->num_errors;
	if(filled_cells==length*length && num_errors==0)
	{/*If the board is solved completely (full with no errors)*/
		printf("The puzzle is already solved, but take the winning message anyway:\n");
		print_puzzle_solved_successfuly();
		return(TRUE);
	}
	/*the board is full with errors or not full*/
	return(FALSE);
}

static BOOL continue_checking_edit(char *delimiters, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * If the command is invalid or there was a problem with the file, prints the matching error message.
	 * Else, executes edit_no_param or load_puzzle_from_file
	 *(both can encounter a mem aloc problem inside, in which case the program is terminated).
	 * Returns TRUE if the command was executed successfully, ERROR if there were
	 * incorrect number of parameters/invalid file format/fopen error)
	 */
	char *path, *p;/*inputs. p for extra invalid parameters*/
	FILE *f;

	/*getting the path from input. It can be NULL if no path was given*/
	path = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(0, "s");
		return(ERROR);
	}
	if(path==NULL)
	{/*No path was given->enter edit mode with an empty 9x9 board*/
		edit_no_param(board);
		return(TRUE);
	}
	if(strstr(path, file_format)==NULL)
	{
		print_invalid_format();
		return(ERROR);
	}
	/*the path is of valid format*/
	f = fopen(path, "r");
	if(f==NULL)
	{
		print_fopen_error();
		return(ERROR);
	}
	load_puzzle_from_file(board, f, EDIT);
	fclose(f);
	return(TRUE);
}


static void continue_checking_mark_errors(char *delimiters, STATUS s, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * If the command is invalid, prints a detailed error message.
	 * Else, executes the set_mark_errors command.
	 */
	char *token, *p;/*input for mark errors as a char and p for extra invalid parameters*/
	float x; /*input for mark errors as a float*/
	BOOL res;

	if(s!=SOLVE)
	{
		print_invalid_mode("Solve mode");
		return;
	}
	token = strtok(NULL, delimiters);
	if(token==NULL)
	{
		print_not_enough_parameters(1, "");
		return;
	}
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(1, "");
		return;
	}

	res = strToNum(token, &x);
	if(res==FALSE){print_nan("");return;}
	if(isWhole(x)==FALSE){print_ni(""); return;}
	if(x!=0 && x!=1)
	{
		print_parameter_notInRange("", "0 or 1");
		return;
	}
	set_mark_errors(x);
	print_board(board,s);
}

static void continue_checking_print_board(char *delimiters, STATUS s, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * If the command is invalid, prints a detailed error message.
	 * Else, executes the print_board command.
	 */
	char *p; /*extra invalid parameters*/
	if(s==INIT)
	{
		print_invalid_mode("Edit or Solve modes");
		return;
	}
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(0, "s");
		return;
	}
	print_board(board, s);
}

static BOOL continue_checking_set(char *delimiters,STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * If the command is invalid, prints a detailed error message.
	 * Else, executes the set command.
	 * Returns TRUE if the board is completely solved in Solve mode, FALSE if not.
	 */
	float X,Y,Z; /*inputs as double*/
	char *x ,*y, *z, *p; /*tmp inputs. p for additional input, which makes the command invalid*/
	BOOL last_cell_filled;

	if(s==INIT)
	{
		print_invalid_mode("Edit and Solve modes");
		return(FALSE);
	}
	/*get parameters from input*/
	x = strtok(NULL,delimiters);
	y = strtok(NULL,delimiters);
	z = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(3, "s");
		return(FALSE);
	}
	if(x==NULL || y==NULL || z == NULL)
	{/*a valid command but not all parameters were given->error*/
		print_not_enough_parameters(3, "s");
		return(FALSE);
	}
	else
	{/* all parameters were given.*/
		if(check_cell_c(x,&X,y,&Y,board->length)==FALSE)
		{/*invalid input. The right error messages were printed already*/
			return FALSE;
		}

		if(check_cell_v(z,&Z,board->length)==FALSE)
		{
			return FALSE;
		}

		if(fixed_or_error(board,X,Y)==1 && s==SOLVE)
		{
			print_cell_fixed();
			return(FALSE);
		}
		last_cell_filled = set(board, X, Y, Z ,s);
		if(s==SOLVE && last_cell_filled)
		{
			/*"set" updates the parameter "num_errors" after an update to the board*/
			if(board->num_errors==0)
			{
				print_puzzle_solved_successfuly();
				return(TRUE);
			}
			else
			{
				print_finished_puzzle_has_errors();
				return(FALSE);
			}
		}
		else
		{
			return(FALSE);
		}
	}
}

static void continue_checking_validate(char *delimiters, STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * If the command is invalid, prints a detailed error message.
	 * Else, executes the validate command.
	 */
	char *p; /*extra invalid parameters*/
	if(s==INIT)
	{
		print_invalid_mode("Edit and Solve modes");
		return;
	}
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(0, "s");
		return;
	}
	if(board->num_errors>0)
	{
		print_erroneous_board("");
		return;
	}
	validate(board);
}

static BOOL continue_checking_guess(char *delimiters,STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes guess if the current mode is Solve,the parameter is valid and the board is not erroneous.
	 * Else, prints a detailed error message.
	 * Returns whether the last cell was filled.
	 */
	char *x,*p; /*input parameter as string. p for extra invalid parameter.*/
	BOOL last_cell_filled;
	float X; /*input parameter as float*/
	BOOL res;

	if(s!=SOLVE)
	{
		print_invalid_mode("Solve mode");
		return FALSE;
	}
	/*get parameters from input*/
	x = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(1, "");
		return FALSE;
	}
	if(x==NULL)
	{/*a valid command but not all parameters were given->error*/
		print_not_enough_parameters(1, "");
		return FALSE;
	}
	else
	{/* all parameters were given.*/
		res = strToNum(x, &X);
		if(res==FALSE){print_nan("");return(FALSE);}

		if(board->num_errors>0)
		{
			print_erroneous_board("");
			return FALSE;
		}
		last_cell_filled = guess(board, X);
		if(last_cell_filled==TRUE)
		{
			if(board->num_errors==0)
			{
				print_puzzle_solved_successfuly_by_computer();
				return(TRUE);
			}
			else
			{
				print_finished_puzzle_has_errors();
				return(FALSE);
			}
		}
		return last_cell_filled;
	}
}

static void continue_checking_generate(char *delimiters, STATUS s, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes generate if the current mode is Edit, the parameters are valid and the board in not erroneous.
	 * Else, prints a detailed error message.
	 */
	char *x,*y,*p;/*inputs. p for extra invalid parameters*/
	float X, Y; /*input parameters as floats*/
	int length;
	int filled_cells;
	BOOL res;

	if(s!=EDIT)
	{
		print_invalid_mode("Edit mode");
		return;
	}
	/*get parameters from board*/
	length = board->length;
	filled_cells = board->filled_cells;
	/*get parameters from input*/
	x = strtok(NULL,delimiters);
	y = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(2, "s");
		return;
	}
	if(x==NULL || y==NULL)
	{/*a valid command but not all parameters were given->error*/
		print_not_enough_parameters(2, "s");
		return;
	}
	else
	{/* all parameters were given.*/
		res = strToNum(x, &X);
		if(res==FALSE){print_nan("first ");return;}
		if(isWhole(X)==FALSE){print_ni("first "); return;}
		if(X<0 || X>(length*length-filled_cells))
		{
			print_parameter_notInRange("first ", "a non-negative integer equal to or smaller\nthan the number of empty cells on the board");
			return;
		}

		res = strToNum(y, &Y);
		if(res==FALSE){print_nan("second ");return;}
		if(isWhole(Y)==FALSE){print_ni("second "); return;}
		if(Y<0 ||Y>length*length)
		{
			print_parameter_notInRange("second ", "a non-negative integer equal to or smaller\nthan the number of total cells on the board");
			return;
		}

		/*all the parameters are legal*/
		/*check the state of the board*/
		if(board->num_errors>0)
		{
			print_erroneous_board("");
			return;
		}
		generate(board, X, Y);
		return;
	}
}

static void continue_checking_undo(char *delimiters, STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes undo if the current mode is Edit or Solve.
	 * Else, prints a detailed error message.
	 * If there are no moves to undo, prints an error message.
	 */
	char *p; /*extra input which makes the command invalid*/
	BOOL cantUndo; /*TRUE if undo couldn't undo any moves*/

	if(s!=EDIT && s!=SOLVE)
	{
		print_invalid_mode("Edit and Solve modes");
		return;
	}
	/*getting extra parameters from input*/
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(0, "s");
		return;
	}
	cantUndo = undo(board, s);
	if(cantUndo)
	{
		print_cant_undo();
		return;
	}
	return;
}

static void continue_checking_redo(char *delimiters, STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes redo if the current mode is Edit or Solve.
	 * Else, prints a detailed error message.
	 * If there are no moves to redo, prints an error message.
	 */
	char *p; /*extra input which makes the command invalid*/
	BOOL cantRedo; /*TRUE if redo couldn't redo any moves*/

	if(s!=EDIT && s!=SOLVE)
	{
		print_invalid_mode("Edit and Solve modes");
		return;
	}
	/*getting extra parameters from input*/
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(0, "s");
		return;
	}
	cantRedo = redo(board, s);
	if(cantRedo)
	{
		print_cant_redo();
		return;
	}
	return;
}

static void continue_checking_save(char *delimiters, STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Doesn't executes save and prints a detailed error message if:
	 * 	the current mode is not Edit or Solve
	 * 	the file format at path is not .txt
	 * 	fopen had an error
	 * 	the board is erroneous and the mode is Edit
	 * 	the board is not solvable and the mode is Edit
	 */
	char *path ,*p; /*input path, p for extra input which makes the command invalid*/
	FILE *f;/*pointer to the file that the board will be saved on*/
	BOOL vald_result;/*validation result*/

	if(s!=EDIT && s!=SOLVE)
	{
		print_invalid_mode("Edit and Solve modes");
		return;
	}
	/*getting parameters from input*/
	path = strtok(NULL,delimiters);
	if(path==NULL)
	{
		print_not_enough_parameters(1, "");
		return;
	}
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(1, "");
		return;
	}
	if(strstr(path, file_format)==NULL)
	{
		print_invalid_format();
		return;
	}
	if(s==EDIT)
	{
		if(board->num_errors>0)
		{
			print_erroneous_board("in the current mode ");
			return;
		}
		/*checking if the board has a solution*/
		printf("Validating board before saving...\n");
		vald_result = validate(board);
		if(vald_result==ERROR)
		{
			return;
		}
		if(vald_result==FALSE)
		{
			print_cant_save();
			return;
		}
	}
	/*the path is correct, and if (the mode is edit and we can save)*/
	f = fopen(path, "w");
	if(f==NULL)
	{
		print_fopen_error();
		return;
	}
	save(board,s,f);
	fclose(f);
	return;
}

static void continue_checking_hint(char *delimiters,STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Checks if all the parameters are given and in range, then executes hint.
	 * Else, prints a detailed error message.
	 */
	float X,Y; /*X Y inputs.*/
	char *x ,*y, *p; /*tmp inputs, to check if they are not NULL. p for additional invalid input*/

	if(s!=SOLVE)
	{
		print_invalid_mode("Solve mode");
		return;
	}
	/*get parameters from input*/
	x = strtok(NULL,delimiters);
	y = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(2, "s");
		return;
	}
	if(x==NULL || y==NULL)
	{/*a valid command but not all parameters were given*/
		print_not_enough_parameters(2, "s");
		return;
	}
	else
	{/* all parameters were given.*/

		if(check_cell_c(x,&X,y,&Y,board->length)==FALSE){return;}

		if(board->num_errors>0)
		{
			print_erroneous_board("");
			return;
		}
		if(fixed_or_error(board,X,Y)==1)/*cell is fixed*/
		{
			print_cell_fixed();
			return;
		}
		if(get(board, X,Y)!=0)
		{
			print_cell_filled();
			return;
		}
		hint(board, X, Y);
		return;
	}
}

static void continue_checking_guess_hint(char *delimiters,STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Checks if all the parameters are given and in range, then executes guess_hint.
	 * Else, prints a detailed error message.
	 */
	float X,Y; /*X Y inputs.*/
	char *x ,*y, *p; /*tmp inputs, to check if they are not NULL. p for additional invalid input*/

	if(s!=SOLVE)
	{
		print_invalid_mode("Solve mode");
		return;
	}
	/*get parameters from input*/
	x = strtok(NULL,delimiters);
	y = strtok(NULL,delimiters);
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(2, "s");
		return;
	}
	if(x==NULL || y==NULL)
	{/*a valid command but not all parameters were given*/
		print_not_enough_parameters(2, "s");
		return;
	}
	else
	{/* all parameters were given.*/

		if(check_cell_c(x,&X,y,&Y,board->length)==FALSE){return;}

		if(board->num_errors>0)
		{
			print_erroneous_board("");
			return;
		}
		if(fixed_or_error(board,X,Y)==1)/*cell is fixed*/
		{
			print_cell_fixed();
			return;
		}
		if(get(board, X,Y)!=0)
		{
			print_cell_filled();
			return;
		}
		guess_hint(board, X, Y);
		return;
	}
}

static void continue_checking_num_solutions(char *delimiters, STATUS s, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes num_solutions if:
	 * 		the current mode is Edit or Solve
	 * 		there are no extra parameters
	 * 		the board is not erroneous
	 * Else, prints an error message.
	 */
	char *p;/*additional invalid input*/

	if(s!=EDIT && s!=SOLVE)
	{
		print_invalid_mode("Edit and Solve modes");
		return;
	}
	/*get parameters from input*/
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{/*at least one parameters to many*/
		print_too_many_parameters(0, "s");
		return;
	}
	if(board->num_errors>0)
	{
		print_erroneous_board("");
		return;
	}
	num_solutions(board);
	return;
}

static BOOL continue_checking_autofill(char *delimiters, STATUS s, sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes autofill if:
	 * 		the current mode is Solve
	 * 		there are no extra parameters
	 * 		the board is not erroneous
	 * Else, prints an error message.
	 * Returns TRUE if the board is completely solved in Solve mode, FALSE if not.
	 */
	char *p;/*extra parameters which make the command invalid*/
	BOOL last_cell_filled;
	if(s!=SOLVE)
	{
		print_invalid_mode("Solve mode");
		return FALSE;
	}
	p = strtok(NULL,delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(0,"s");
		return FALSE;
	}
	if(board->num_errors>0)
	{
		print_erroneous_board("");
		return FALSE;
	}
	last_cell_filled = autofill(board,s);
	if(last_cell_filled)
	{
		if(board->num_errors==0)
		{
			print_puzzle_solved_successfuly_by_computer();
			return(TRUE);
		}
		else
		{
			print_finished_puzzle_has_errors();
			return(FALSE);
		}
	}
	return last_cell_filled;
}

static void continue_checking_reset(char *delimiters, STATUS s,sudoku_board *board)
{
	/*
	 * Helper function for get_and_execute_command.
	 * Executes reset if the current mode is Edit or Solve and there are no extra parameters.
	 * Else, prints an error message.
	 */
	char *p;/*extra parameters which make the command invalid*/

	if(s==INIT)
	{
		print_invalid_mode("Edit and Solve modes");
		return;
	}
	p = strtok(NULL, delimiters);
	if(p!=NULL)
	{
		print_too_many_parameters(0,"");
		return;
	}
	reset(board,s);
	return;
}

STATUS get_and_execute_command(sudoku_board *board,STATUS s)
{
	/*
	 * Gets input from user and checks for (in this order):
	 * -Command name is correct. Command names are case sensitive.
	 * 		The commands are: solve X
	 * 						  edit [X] (X is optional)
	 * 						  mark_errors X
	 * 						  print_board
	 * 						  set X Y Z
	 * 						  validate
	 * 						  guess X
	 * 						  generate X Y
	 * 						  undo
	 * 						  redo
	 * 						  save X
	 * 						  hint X Y
	 * 						  guess_hint X Y
	 * 						  num_solutions
	 * 						  autofill
	 * 						  reset
	 * 						  exit
	 * -The command is available in the correct mode.
	 * -The command has the correct number of parameters. Extra parameters makes the command invalid.
	 * -All parameters are correct. For each one checks:
	 * 								a.It is in the correct range.
	 * 								b.Its value is legal for the current board state.
	 * -The board is valid for the command.
	 * -The command is executed successfully.
	 * If any error is found, a detailed error message is printed.
	 * NOTE: if an input is a valid command + a delimiter at the end, it is still a valid command.
	 */
	int MAX_CHAR_CMD = 259; /*max characters in a valid command is 256. +1 for "\n", +1 for "\0", +1 for a command with more than 256 characters which is invalid*/
	char *delimiters = " \t\r\n";
	char *input = NULL;
	size_t buffsize = 0; /*input length in bytes*/
	int in_len; /*number of characters in input*/
	char *token;
	BOOL switch_to_init; /*TRUE iff the sudoku puzzle is solved completely in solve mode*/
	BOOL execution_result;/*TRUE iff the command was executed successfully, ERROR otherwise*/

	printf("Enter a command:\n");
	if((in_len = getline(&input, &buffsize, stdin))!=-1)/*not EOF or read error*/
	{
		if(in_len>=MAX_CHAR_CMD-1) /*in_len doesn't include the null terminator*/
		{
			printf("Invalid Command: The command has more than 256 characters.\n");
			free(input);
			return(s);
		}
		token = strtok(input,delimiters);
		if(token!=NULL)
		{
			if(strcmp(token,"solve")==0)
			{
				switch_to_init = continue_checking_solve(delimiters, board);
				free(input);
				if(switch_to_init==TRUE)
				{/*the loaded puzzle is completely solved*/
					return(INIT);
				}
				if(switch_to_init==ERROR)
				{/*the command was invalid or couldn't open file*/
					return(s);
				}
				return(SOLVE);
			}
			else if(strcmp(token,"edit")==0)
			{
				execution_result = continue_checking_edit(delimiters, board);
				free(input);
				if(execution_result==ERROR)
				{/*when the command is invalid we do not switch the mode*/
					return(s);
				}
				return(EDIT);
			}
			else if(strcmp(token,"mark_errors")==0)
			{
				continue_checking_mark_errors(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"print_board")==0)
			{
				continue_checking_print_board(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"set")==0)
			{
				switch_to_init = continue_checking_set(delimiters, s, board);
				free(input);
				if(switch_to_init==TRUE)
				{/*The game is finished in Solve mode, so the game mode is set to INIT*/
					return(INIT);
				}
				return(s);
			}
			else if(strcmp(token,"validate")==0)
			{
				continue_checking_validate(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"guess")==0)
			{
				switch_to_init = continue_checking_guess(delimiters, s, board);
				free(input);
				if(switch_to_init==TRUE)
				{/*The game is finished in Solve mode, so the game mode is set to INIT*/
					return(INIT);
				}
				return(s);
			}
			else if(strcmp(token,"generate")==0)
			{
				continue_checking_generate(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"undo")==0)
			{
				continue_checking_undo(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"redo")==0)
			{
				continue_checking_redo(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"save")==0)
			{
				continue_checking_save(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"hint")==0)
			{
				continue_checking_hint(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"guess_hint")==0)
			{
				continue_checking_guess_hint(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"num_solutions")==0)
			{
				continue_checking_num_solutions(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"autofill")==0)
			{
				switch_to_init = continue_checking_autofill(delimiters, s, board);
				free(input);
				if(switch_to_init)
				{/*The game is finished in Solve mode, so the game mode is set to INIT*/
					return(INIT);
				}
				return(s);
			}
			else if(strcmp(token,"reset")==0)
			{
				continue_checking_reset(delimiters, s, board);
				free(input);
				return(s);
			}
			else if(strcmp(token,"exit")==0)
			{
				free(input);
				return(EXIT);
			}
			/*Command name is incorrect*/
			printf("Invalid command: command name is incorrect. Note: command names are case sensitive.\n");
			free(input);
			return(s);
		}
		else
		{/*meaning empty command -> ignored*/
			free(input);
			return(s);
		}
	}
	else
	{/*EOF or read error*/
		printf("Error: EOF or a read error reached.\n");
		free(input);
		return(EXIT);
	}
}
