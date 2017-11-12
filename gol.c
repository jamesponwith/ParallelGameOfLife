/**
 * File: gol.c
 *
 * Starter code for COMP280 Project 6 ("Game of Life")
 *
 * Replace this comment with your top-level comment with your name and a
 * description of this program.
 */

#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

typedef struct {
	int num_rows;
	int num_cols;
	int num_its;
	int num_pairs;
	int size;
} BoardSpecs;

struct timeval start_time;
	

// Forward function declarations
void printError(); 
int* initBoard(char *ascii_filename, BoardSpecs *b);
void printBoardSpecs(BoardSpecs *b); 
void printBoard(int *board, BoardSpecs *bs); 
int getIndex(int row, int col, int n_cols) { return row*n_cols+col; }

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: %s *rest of the args*", executable_name);
}

int main(int argc, char *argv[]) {
	BoardSpecs *bs = malloc(sizeof(BoardSpecs));
	char *ascii_filename;
	int *board;
	//int verbose = 0;

	opterr = 0;
	int c = -1;

	while ((c = getopt(argc, argv, "c:")) != -1) {
		switch(c) {
			case 'c':
				ascii_filename = optarg;
				break;
			case 'v':
				//verbose = 1;
				break;
			case 'l':
				//advanced
				break;
			case 'n':
				//advanced
				break;
			default:
				usage(argv[0]);
				exit(1);
		} }
	
	// Step 2: Read in the configuration file and use it to initialize your game
	// board. Write a function to do this for you.
	
	board = initBoard(ascii_filename, bs);
	printBoardSpecs(bs);
	printf("\n");
	printBoard(board, bs);
	// Step 3: Start your timer
	
	// Step 4: Simulate for the required number of steps.
	// Again, you should put this in its own function
	
	// Step 5: Stop your timer, calculate amount of time simulation ran for and
	// then print that out.
	free(board);
	free(bs);
	return 0;
}
/*
 A live cell with zero or one live neighbors dies from loneliness.
 A live cell with four or more live neighbors dies due to overpopulation.
 A dead cell with exactly three live neighbors becomes alive.
 */

/**
 * Initializes the board for the game.
 * @param *fp The file object with board specifications
 * @param *bs The board struct with its specs
 * @return the 2d board array, in a 1d int pointer array
 */
int* initBoard(char* ascii_filename, BoardSpecs *bs) {
	FILE *fp = fopen(ascii_filename, "r");
	int *board;
	int col;
	int row;

	if ((fp) == NULL) {
		printf("No such file\n");
		exit(1);
	}

	fscanf(fp, "%d", &bs->num_rows);
	fscanf(fp, "%d", &bs->num_cols);
	fscanf(fp, "%d", &bs->num_its);
	fscanf(fp, "%d", &bs->num_pairs);
	bs->size = bs->num_cols * bs->num_rows;

	board = (int*) calloc((bs->num_rows * bs->num_cols), sizeof(int));

	// spots for initial state
	for (int i= 0; i < bs->num_pairs; i++) {
		fscanf(fp, "%d %d", &col, &row); 
		int index = getIndex(row, col, bs->num_cols);
		board[index] = 1;
	}

	fclose(fp);
	return board;
}	

/**
 * Prints out the board
 * @param *board The game board
 * @param *bs The board's specs
 */
void printBoard(int *board, BoardSpecs *bs) {
	for (int i = 0; i < bs->size; i++) {
		if (board[i] == 0) {
			printf(". ");
		}
		else {
			printf("@ ");
		}
		if (((i + 1) % bs->num_cols) == 0) {
			printf("\n");
		}
	}
}

/**
 * Prints out board specs
 * @param *bs the board's specs
 */
void printBoardSpecs(BoardSpecs *bs) {
	printf("Num rows: %d\n", bs->num_rows); 
	printf("Num cols: %d\n", bs->num_cols);  
	printf("Num its:  %d\n", bs->num_its); 
	printf("Pairs:    %d\n", bs->num_pairs);
}

