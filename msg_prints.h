#ifndef ERROR_MSG_PRINTS_H_
#define ERROR_MSG_PRINTS_H_

/*
 *This module contains all message prints necessary to the sudoku game code.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "definitions.h"

void print_memory_allocation_error_msg(char *function_name);

void print_EOF_or_read_error();

void print_too_many_iterations(char *function_name);

void print_not_enough_parameters(int count, char *s);

void print_too_many_parameters(int count, char *s);

void print_invalid_mode(char *validModes);

void print_nan(char* place);

void print_ni(char* place);

void print_parameter_notInRange(char* number, char *correctRange);

void print_erroneous_board(char *add);

void print_cant_undo();

void print_cant_redo();

void print_fopen_error();

void print_cant_save();

void print_invalid_format();

void print_cell_fixed();

void print_cell_filled();

void print_cant_hint();

void print_puzzle_solved_successfuly();

void print_finished_puzzle_has_errors();

/*If autofill or guess solves the puzzle*/
void print_puzzle_solved_successfuly_by_computer();

void print_no_change_in_autofill();

void print_no_change_in_guess(float X);

void print_NULL_scores();

void print_no_wanted_scores();

void print_no_change_in_generate(int X, int Y);

void print_no_change_in_reset();

void print_reset_done();

#endif
