CC = gcc
OBJS = main.o main_aux.o parser.o game.o solver.o SPBufferset.o linked_list.o doubly_linked_list.o msg_prints.o move_list.o matrix.o 
EXEC = sudoku-console
COMP_FLAGS = -ansi -O3 -Wall -Wextra -Werror -pedantic-errors
GUROBI_COMP = -I/usr/local/lib/gurobi563/include
GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $@ -lm
main.o: main.c main_aux.h parser.h SPBufferset.h definitions.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
main_aux.o: main_aux.c main_aux.h msg_prints.h definitions.h matrix.h move_list.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
parser.o: parser.c parser.h game.h definitions.h msg_prints.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
solver.o: solver.c solver.h definitions.h matrix.h msg_prints.h linked_list.h game.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
SPBufferset.o: SPBufferset.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
linked_list.o: linked_list.c linked_list.h msg_prints.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
msg_prints.o: msg_prints.h msg_prints.c definitions.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
doubly_linked_list.o: doubly_linked_list.h doubly_linked_list.c definitions.h msg_prints.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
move_list.o: move_list.h move_list.c doubly_linked_list.h definitions.h msg_prints.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
matrix.o: matrix.c matrix.h definitions.h msg_prints.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
game.o: game.c game.h linked_list.h move_list.h main_aux.h solver.h msg_prints.h definitions.h matrix.h doubly_linked_list.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c

all: $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC)