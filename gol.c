/**
 * File: gol.c
 *
 * Starter code for COMP280 Project 6 ("Game of Life")
 *
 * Replace this comment with your top-level comment with your name and a
 * description of this program.
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
} BoardSpecs; struct timeval start_time;
struct timeval curr_time;
	
int left = 0;

// Forward function declarations
int* initBoard(char *ascii_filename, BoardSpecs *b);
int to1d(int row, int col, BoardSpecs *bs);

int numNeighbors(int *board, BoardSpecs *bs, int row, int col); 

void printBoardSpecs(BoardSpecs *b); 
void printBoard(int *board, BoardSpecs *bs); 
void updateBoard(int *board, BoardSpecs *bs); 
void sim(int *board, BoardSpecs *bs); 

void printError(); 
/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: -c <%s> *rest of the args*", executable_name);
}

int main(int argc, char *argv[]) {
	BoardSpecs *bs = malloc(sizeof(BoardSpecs));
	char *ascii_filename = NULL;
	int *board = NULL;
	//int ret = 0;
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
		}
   	}
	
	board = initBoard(ascii_filename, bs);
	//printBoardSpecs(bs);
	//printf("\n");
	//printBoard(board, bs);
	// Step 3: Start your timer
	//ret = gettimeofday(&start_time, NULL);
	//printf("\n%d\n", start_time);	
	//printf("\n%d\n", curr_time);	
	

	// Step 4: Simulate for the required number of steps.
	// Again, you should put this in its own function
	
	sim(board, bs);
	
	// Step 5: Stop your timer, calculate amount of time simulation ran for and
	// then print that out.
	//ret = gettimeofday(&curr_time, NULL);
	//printf("%d", ret);
	//printf("\n%d\n", left);
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
	int num_neighbors;
	int *tmp_board = (int*) calloc((bs->num_rows * bs->num_cols), sizeof(int)); 

	for (int i = 0; i < bs->num_rows; i++) {
		for (int j = 0; j < bs->num_cols; j++) {
			num_neighbors = numNeighbors(board, bs, i, j);	
			if (board[to1d(i,j,bs)] == 0) {
				if (num_neighbors == 3) {
					tmp_board[i] = 1;
				}
			}
			else if (board[i] == 1) {
				if (!(num_neighbors <= 1 || num_neighbors >= 4)) {
					tmp_board[i] = 1;
				}
			}
		}
	}
	for (int i = 0; i < bs->size; i++) {
		board[i] = tmp_board[i];
	}
	free(tmp_board);
}

/**
 * Gets the number of neighbors for a given cell 
 * @param *board The game board
 * @param *bs The board's specificaitons 
 * @param pos The position of the cell in the 1D array
 *
 * @return the cell's number of neighbors 
 */
int numNeighbors(int *board, BoardSpecs *bs, int row, int col) {
	int temp_row;
	int temp_col;
	int alive = 0;
	// up left
	temp_row = row - 1;
	temp_col = col - 1;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}

	// above
	temp_row = row - 1;
	temp_col = col;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}

	// up right
	temp_row = row - 1;
	temp_col = col + 1;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}

	// right
	temp_row = row;
	temp_col = col + 1;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}

	// down right
	temp_row = row + 1;
	temp_col = col + 1;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}
	// below
	temp_row = row + 1;
	temp_col = col;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}
	// down left
	temp_row = row + 1;
	temp_col = col - 1;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}
	// left
	temp_row = row;
	temp_col = col - 1;
	if (board[to1d(temp_row, temp_col, bs)] == 1) {
		alive++;
	}
	return alive;
}

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
		int index = to1d(row, col, bs);
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

		if (board[i] == 0) { printf(". "); }
		else { printf("@ "); }

		if (((i + 1) % bs->num_cols) == 0) { printf("\n"); }
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
/**
 * Simulates the game on terminal window
 * @param *board The game board
 * @param *bs The board's specificatoins
 */
void sim(int *board, BoardSpecs *bs) {
	system("clear");
	for (int i = 0; i < bs->num_its; i++) {
		printBoard(board, bs);
		updateBoard(board, bs); 
		usleep(100000);	
		if (i == bs->num_its - 1) {
			return;
		}
		system("clear");
	}
}

int to1d(int row, int col, BoardSpecs *bs) { 
	if (row < 0) {
		row = bs->num_rows - 1;
	}
	else if (row == bs->num_rows) {
		row = 0;
	}
	if (col < 0) {
		col = bs->num_cols - 1;
	}
	else if (row == bs->num_cols) {
		col = 0;
	}
	return row*bs->num_cols+col; 
}
