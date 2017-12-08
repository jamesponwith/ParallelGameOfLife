/**
 * File: gol.c
 * Author: Patrick Hall
 *
 * This program simulates Conway's game of life.
 * The starting board specifications are passed through the command line
 * argument.
 * The program will run until the specified number of iterations has passed 
 */

#define _XOPEN_SOURCE 600

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

void printError(); 
void printBoardSpecs(BoardSpecs *b); 
int to1d(int row, int col, BoardSpecs *bs);
void printBoard(int *board, BoardSpecs *bs); 
void updateBoard(int *board, BoardSpecs *bs); 
void sim(int *board, BoardSpecs *bs, int verbose); 
int* initBoard(char *ascii_filename, BoardSpecs *b);
int numAlive(int *board, BoardSpecs *bs, int row, int col); 
void timeval_subtract(struct timeval *result, 
					struct timeval *end, struct timeval *start); 

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: -c <%s> -v <verbose mode>\n", executable_name);
}

int main(int argc, char *argv[]) {
	int verbose = 0;
	char *ascii_filename = NULL;

	opterr = 0;
	int c = -1; 

	while ((c = getopt(argc, argv, "c:v")) != -1) {
		switch(c) {
			case 'c':
				ascii_filename = optarg;
				if (ascii_filename == NULL) {
					printf("You must specify a filename\n");
					exit(1);
				}
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
   	}
	BoardSpecs *bs = malloc(sizeof(BoardSpecs));

	int *board = initBoard(ascii_filename, bs);

	struct timeval start_time, curr_time, result;

	gettimeofday(&start_time, NULL); 	// get start time before starting game

	sim(board, bs, verbose); 				// start game

	gettimeofday(&curr_time, NULL); 	// check time after game

	timeval_subtract(&result, &curr_time, &start_time); // calculate time for program

	printf("Total time for %d iterations of %dx%d world is ", 
					bs->num_its, bs->num_cols, bs->num_rows);
	printf("%ld.%06ld\n", result.tv_sec, result.tv_usec);	

	free(board);
	free(bs);
	return 0;
}

/**
 * Updates the values in the board array
 * base off the rules of the game
 *
 * @param *board The board
 * @param *bs The board's specs
 */
void updateBoard(int *board, BoardSpecs *bs) {
	int num_alive;
	int *tmp_board = (int*) calloc((bs->num_rows * bs->num_cols), sizeof(int)); 

	// determine new state of board
	for (int i = 0; i < bs->num_rows; i++) {
		for (int j = 0; j < bs->num_cols; j++) {
			//number of alive surrounding cells
			num_alive = numAlive(board, bs, i, j);	
			if (board[to1d(i,j,bs)] == 0) {
				if (num_alive == 3) {
					tmp_board[to1d(i,j,bs)] = 1;
				}
				else {
					tmp_board[to1d(i,j,bs)] = 0;
				}
			}
			else {
				if ((num_alive <= 1 || num_alive >= 4)) {
					tmp_board[to1d(i,j,bs)] = 0;
				}
				else {
					tmp_board[to1d(i,j,bs)] = 1;
				}
			}
		}
	}

	// copy values of tmp_board to board
	for (int i = 0; i < bs->num_rows; i++) {
		for (int j = 0; j < bs->num_cols; j++) {
			board[to1d(i,j,bs)] = tmp_board[to1d(i,j,bs)];
		}
	}
	free(tmp_board);
}

/**
 * Gets the number of surrounding alive cells
 * @param *board The game board
 * @param *bs The board's specificaitons 
 * @param pos The position of the cell in the 1D array
 *
 * @return the number of surrounding alive cells 
 */
int numAlive(int *board, BoardSpecs *bs, int row, int col) {
	int alive = 0;
	//top row
	alive += board[to1d(row+1, col-1, bs)];
	alive += board[to1d(row+1, col+0, bs)];
	alive += board[to1d(row+1, col+1, bs)];

	//left and right
	alive += board[to1d(row, col-1, bs)];
	alive += board[to1d(row, col+1, bs)];

	//bottom row
	alive += board[to1d(row-1, col-1, bs)];
	alive += board[to1d(row-1, col+0, bs)];
	alive += board[to1d(row-1, col+1, bs)]; 
	return alive;
}

/**
 * Initializes the board for the game.
 *
 * @param *fp The file object with board specifications
 * @param *bs The board struct with its specs
 *
 * @return the 2d board array, in a 1d int pointer array
 */
int* initBoard(char* ascii_filename, BoardSpecs *bs) {
	FILE *fp = fopen(ascii_filename, "r");
	//int *board;

	if ((fp) == NULL) {
		printf("No such file\n");
		exit(1);
	}

	fscanf(fp, "%d", &bs->num_rows);
	fscanf(fp, "%d", &bs->num_cols);
	fscanf(fp, "%d", &bs->num_its);
	fscanf(fp, "%d", &bs->num_pairs);
	bs->size = bs->num_cols * bs->num_rows;

	int *board = (int*) calloc((bs->num_rows * bs->num_cols), sizeof(int));

	// spots for initial state
	int col, row;
	for (int i= 0; i < bs->num_pairs; i++) {
		fscanf(fp, "%d %d", &col, &row); 
		int index = to1d(row, col, bs);
		board[index] = 1;
	}

	fclose(fp);
	return board;
}	

/**
 * Prints out the board
 *
 * @param *board The game board
 * @param *bs The board's specs
 */
void printBoard(int *board, BoardSpecs *bs) {
	for (int i = 0; i < bs->size; i++) {
		if (board[i] == 0) { 
			printf("."); 
		}
		else { 
			printf("@"); 
		}
		if (((i + 1) % bs->num_cols) == 0) { 
			printf("\n"); 
		}
	}
}

/**
 * Prints out board specs
 *
 * @param *bs the board's specs
 */
void printBoardSpecs(BoardSpecs *bs) {
	printf("Num rows: %d\n", bs->num_rows); 
	printf("Num cols: %d\n", bs->num_cols);  
	printf("Num its:  %d\n", bs->num_its); 
	printf("Pairs:    %d\n", bs->num_pairs);
}

/**
 * Simulates the game on terminal window
 *
 * @param *board The game board
 * @param *bs The board's specificatoins
 */
void sim(int *board, BoardSpecs *bs, int verbose) {
	if (verbose == 1) {
		system("clear");
	}
	for (int i = 0; i <= bs->num_its; i++) {
		updateBoard(board, bs); 
		if (verbose == 1) {
			printf("Time step: %d\n", i);
			printBoard(board, bs);
			usleep(100000 * 2);
			if (!(i == bs->num_its)) {
				system("clear");
			}
		}
	}
}

/**
 * Converts 2d array coordinates to 1d array index
 *
 * @param row The row index
 * @param col The col incex
 * @param *bs The board specifications
 *
 * @return the 1d index of the board
 */
int to1d(int row, int col, BoardSpecs *bs) { 
	if (row < 0) {
		row = bs->num_rows + row;
	}
	else if (row >= bs->num_rows) {
		row = row % bs->num_rows;
	}
	if (col < 0) {
		col = bs->num_cols + col;
	}
	else if (col >= bs->num_cols) {
		col = col % bs->num_cols;
	}
	return row*bs->num_cols+col; 
}

/**
 * Calculates the amount of time between start and end
 * Taken from Dr Sat's starter code/GNU documentation
 *
 * @param *start The start time
 * @param *end The end time
 * @param *result The resulting time difference
 */
void timeval_subtract(struct timeval *result, 
					struct timeval *end, struct timeval *start) 
{
	// Perform the carry for later subracting by updating start
	if (end->tv_usec < start->tv_usec) {
		int nsec = (start->tv_usec - end->tv_usec) / 1000000 + 1;
		start->tv_usec -= 1000000 * nsec;
		start->tv_sec += nsec;
	}
	if (end->tv_usec - start->tv_usec > 1000000) {
		int nsec = (end->tv_usec - start->tv_usec) / 1000000;
		start->tv_usec += 1000000 * nsec;
		start->tv_sec -= nsec;
	}

	// Compute the time remaining to wait.tv_usec is certainly positive
	result->tv_sec = end->tv_sec - start->tv_sec;
	result->tv_usec = end->tv_usec - start->tv_usec;
}
