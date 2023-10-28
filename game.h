#ifndef __GAME_H__
#define __GAME_H__


#include "linked_list.h"

/*
 *This module encapsulates the Sudoku puzzle game and the Sudoku board.
 *It will be used to print, store and manipulate the board, or when a property of the board
 *is needed, such as the board's stored solution, fixed cells, legal values for a cell and many more :)
 */


/*
 * Gets the (i-1,j-1) cell's value of the board.
 * returns: the cells value if valid. ERROR if failed, 0 if cell is empty.
 */
int get(sudoku_board *board, int i, int j);

/*
 * Sets the (i-1,j-1) cell to value. if value is 0, clears the cell.
 * returns TRUE if the last cell on the board was filled, FALSE otherwise.
 * Assumes board is not NULL, i,j are in the correct range.
 * Assumes that if called in solve mode, the cell is not fixed.
 * An erroneous value (equal to one of the neighbors) is allowed.
 * updates:
 * board.filled_cells.
 * board.fixed_or_error when an illegal value was set.
 * board.num_errors to the new number of errors.
 * recent_move to this set move and destroys any move that was after recent_move before this command was called.
 */
BOOL set(sudoku_board *board, int i, int j, int value, STATUS status);

/*
 * Validates that the current state of the board is solvable using ILP.
 * Returns and prints if validation passed(TRUE) or failed(FALSE).
 * If validation passed, updates the stored solution of board.
 * Assumes the mode is Edit or Solve and that the board is not erroneous.
 */
BOOL validate(sudoku_board *board);

/*
 * Runs ILP to solve the board. If the board is unsolvable prints an error message.
 * Else, prints to the user the value at column x and row y of the solution.
 * Assumes the mode is Solve, the board is not erroneous,
 * cell <x,y> is not fixed and doesn't contain a value.
 */
void hint(sudoku_board *board, int x, int y);

/*
 * Gets the (i-1,j-1) fixation state.
 * returns: 2 if the cell is erroneous, 1 if the cell is fixed, 0 if neither. ERROR if failed.
 */
int fixed_or_error(sudoku_board *board, int i,int j);

/*
 * If sol is FALSE:
 * Return True iff "value" is a legal value for column i row j of board (meaning it is different from all neighbors).
 * If sol is TRUE:
 * Return True iff "value" is a legal value for column i row j of board.solution (meaning it is different from all neighbors).
 */
BOOL is_legal(sudoku_board *board, int i, int j, int value, BOOL sol);

/*
 * Prints the board according to page 5 in the pdf.
 */
void print_board(sudoku_board *board,STATUS status);

/*
 * Returns a LINKEDLIST of legal values for the cell at row i-1 and column j-1.
 */
LINKEDLIST get_legal_values(sudoku_board *board, int i, int j,BOOL sol);

/*
 * returns the value of the i,j cell in solution.
 * 1<=i,j<=length and board is initialized are assumed.
 * If an error occurs or values are illegal, returns ERROR.
 */
int get_solution(sudoku_board *board, int i, int j);

/*
 * Loads file into board. If there is data in board, it is replaced.
 * Frees matrices of board if their sizes don't match with the new sizes, and allocates new matrices.
 * Uses exit() if encountered a memory allocation error and prints the error message.
 * Assumes file is an already opened stream on reading mode. Does not close the stream.
 * For practical use, this method will not accept a puzzle with horizontal/vertical block length
 * above 5. In this case, a message will be printed and the playing board will stay the same.
 */
void load_puzzle_from_file(sudoku_board *board, FILE *file, STATUS s);

/*
 * Changes data in board to a 9 by 9 empty sudoku board.
 * Frees matrices of board if their sizes don't match a 9x9 normal board, and allocates new matrices.
 * If encountered a memory allocation error, prints the error message and terminates the program.
 * Otherwise, prints the board to the user.
 */
void edit_no_param(sudoku_board *board);

/*
 * Sets the global parameter "mark_errors" to x.
 * Assumes x is 1 or 0.
 */
void set_mark_errors(int x);

/*
 * Guesses a solution to the current board using LP with threshold X.
 * Fills all cell values with a score of X or greater. If several values hold for the same cell,
 * randomly chooses one according to the score(a score of 0.6 has double the chance of 0.3).
 * Doesn't fill illegal values created along the way.
 * Prints the board when finished.
 * Returns whether the last cell was filled.
 * Assumes the mode is Solve, X is in the right range and the board is not erroneous.
 */
BOOL guess(sudoku_board *board, float X);

/*
 * Runs LP to solve the board. If the board is unsolvable prints an error message.
 * Else, prints to the user all legal values for column x and row y of the board,
 * and their score (prints only values with scores greater than 0)..
 * Assumes the mode is Solve, the board is not erroneous,
 * cell <x,y> is not fixed and doesn't contain a value.
 */
void guess_hint(sudoku_board *board, int x, int y);

/*
 * Generates a puzzle by randomly filling X empty cells on the current board with legal values,
 * running ILP to solve the board, and then clearing all but (any) Y random cells.
 * Assumes X and Y are legal values and in the correct range, plus the board is not erroneous.
 * The method prints the board if the command was executed successfully.
 */
void generate(sudoku_board *board, int X, int Y);

/*
 * Undo previous move done by the user.
 * Sets the current move pointer to the previous move and updates the board accordingly.
 * This does not add or remove any item to/from the list.
 * If there are no moves to undo, Returns TRUE. Returns FALSE otherwise.
 * If there is a move to undo, clearly prints to the user the change that was made.
 */
BOOL undo(sudoku_board *board, STATUS s);

/*
 * Redo previous move done by the user.
 * Sets the current move pointer to the next move and updates the board accordingly.
 * This does not add or remove any item to/from the list.
 * If there are no moves to redo, Returns TRUE. Returns FALSE otherwise.
 * If there is a move to redo, clearly prints to the user the change that was made.
 */
BOOL redo(sudoku_board *board, STATUS s);

/*
 * Saves the current game board to the specified file f, according to the file format in page 13 in the pdf.
 * Assumes f points to an existing valid file, the mode is right and the board is valid.
 * In Edit mode, cells containing values are marked as "fixed" in the saved file.
 * Prints that the board was successfully saved.
 */
void save(sudoku_board *board, STATUS s, FILE *f);

/*
 * Runs an exhaustive backtracking algorithm for the current board to find how many solutions
 * there are, and then prints the result.
 */
void num_solutions(sudoku_board *board);

/*
 * Automatically fills cells which contained a single legal value before this command was executed.
 * Assumes the mode is Solve and the board is not erroneous.
 * The method prints the board if the command was executed successfully.
 * Some cells may be filled with values that are erroneous (two neighbors with the same value).
 * Returns whether the last cell of the board was filled.
 */
BOOL autofill(sudoku_board *board, STATUS s);

/*
 * Undo all moves, reverting the board to its original state.
 * Assumes the mode is Edit or Solve.
 * The undo/redo list in not cleared, the pointer is just moved to the head.
 * The method prints the board once all the undo's are done.
 */
void reset(sudoku_board *board, STATUS s);

/*
 * updates all erroneous values that might have changed due to updating the (i-1,j-1) cell
 */
void update_err_changed_cell(sudoku_board *board, int i, int j,STATUS s);

/*
 * Updates the erroneous markings over all of the board.
 * Assumes board is initialized.
 */
void update_err_board(sudoku_board *board, STATUS s);

#endif /*__GAME_H__*/
