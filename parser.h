#ifndef __PARSER_H__
#define __PARSER_H__
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "msg_prints.h"
#include "game.h"
#include "main_aux.h"


/*
 * This module gets commands from the user and executes them.
 * Prints detailed error messages when it is needed.
 */

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
 */
STATUS get_and_execute_command(sudoku_board *board, STATUS s);

#endif /*__PARSER_H__*/
