#include "msg_prints.h"

/*
 *This module contains all message prints necessary to the sudoku game code.
 */

void print_memory_allocation_error_msg(char *function_name)
{
	printf("Error: Memory allocation in %s has failed :(. Terminating program...\n", function_name);
}

void print_EOF_or_read_error()
{
	printf("Error: EOF or a read error reached.\n");
}

void print_too_many_iterations(char *function_name)
{
	printf("Error: Too many iterations for %s.\n", function_name);
}

void print_not_enough_parameters(int count, char *s)
{
	printf("Invalid command: Not enough parameters were given. This command needs %d parameter%s.\n", count, s);
}

void print_too_many_parameters(int count, char *s)
{
	printf("Invalid command: Too many parameters were given. This command needs %d parameter%s.\n", count, s);
}

void print_invalid_mode(char *validModes)
{
	printf("Invalid command: This command is unavailable in the current mode.\nIt is only available in %s.\n",validModes);
}

void print_nan(char* place)
{
	printf("Invalid command: The %sparameter needs to be a number.\n", place);
}

void print_ni(char* place)
{
	printf("Invalid command: The %sparameter needs to be an integer.\n", place);
}

void print_parameter_notInRange(char* number, char *correctRange)
{
	printf("Invalid command: The %sparameter is not in the correct range.\nIt can be %s.\n",number, correctRange);
}

void print_erroneous_board(char *add)
{
	printf("Invalid command: This command cannot be executed %swhile the board contains errors.\n", add);
}

void print_cant_undo()
{
	printf("Invalid command: There are no moves to undo.\n");
}

void print_cant_redo()
{
	printf("Invalid command: There are no moves to redo.\n");
}

void print_fopen_error()
{
	printf("Error: Could not open file. %s.\n", strerror(errno));
}

void print_cant_save()
{
	printf("Error: boards with no solution cannot be saved in Edit mode.\n");
}

void print_invalid_format()
{
	printf("Invalid command: the file format must be of type \"%s\"\n",file_format);
}

void print_cell_fixed()
{
	printf("Error: cell is fixed.\n");
}

void print_cell_filled()
{
	printf("Error: cell already has a value.\n");
}

void print_cant_hint()
{
	printf("Error: Board is unsolvable, can't hint.\n");
}

void print_puzzle_solved_successfuly()
{
	printf("Puzzle was solved successfully! :D. Setting game mode to Init.\n");
}

void print_finished_puzzle_has_errors()
{
	printf("Solution contains errors. You must undo moves to continue solving.\n");
}

void print_puzzle_solved_successfuly_by_computer()
{/*If autofill solves the puzzle*/
	printf("Puzzle was solved successfully by the computer! Yay computer! :D.\nSetting game mode to Init.\n");
}

void print_no_change_in_autofill()
{
	printf("No change: there are no cells with one legal value.\n");
}

void print_no_change_in_guess(float X)
{
	printf("No change: there are no cells with an LP score of %f or greater.\n", X);
}

void print_NULL_scores()
{
	printf("Error: the board is not solvable with LP.\n");
}

void print_no_wanted_scores()
{
	printf("There are no values with scores greater than 0 for this cell.\n");
}

void print_no_change_in_generate(int X, int Y)
{
	printf("No change: %d %d???.\n", X, Y);
}

void print_no_change_in_reset()
{
	printf("No change: The board is already at it's loading state.\n");
}

void print_reset_done()
{
	printf("The board has been successfully reset to it's loading state.\n");
}

