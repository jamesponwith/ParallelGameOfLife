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
} Board;

// Forward function declarations
void printError(); 
void initBoard(char *ascii_filename, Board *b);
void printBoardSpecs(Board *b); 

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: %s *rest of the args*", executable_name);
}

int main(int argc, char *argv[]) {
	Board *board = malloc(sizeof(Board));
	char *ascii_filename;
	//int verbose = 0;

	// Step 1: Parse command line args (I recommend using getopt again).
	// You need to support the "-c" and "-v" options for the basic requirements.
	// The advanced requirements require you to add "-l" and "-n" options.
	
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
				break;
			case 'n':
				break;
			default:
				usage(argv[0]);
				exit(1);
		}

	}
	
	// Step 2: Read in the configuration file and use it to initialize your game
	// board. Write a function to do this for you.
	
	initBoard(ascii_filename, board);
	printBoardSpecs(board);
	// Step 3: Start your timer
	
	// Step 4: Simulate for the required number of steps.
	// Again, you should put this in its own function
	
	// Step 5: Stop your timer, calculate amount of time simulation ran for and
	// then print that out.
	free(board);
	return 0;
}
/**
	num rows
	num cols
	num its 
	num of following coordinate pairs; set each (c, r) value to 1
	c r
	c r 
	...
 */

/**
 * Initializes the board for the game.
 *
 * @param *fp The file object with board specifications
 * @param *board The board struct with its specs
 */
void initBoard(char* ascii_filename, Board *b) {
	FILE *fp = fopen(ascii_filename, "r");
	if ((fp) == NULL) {
		printf("No such file\n");
		exit(1);
	}
	
	if((fscanf(fp, "%d", &b->num_rows) ||
		fscanf(fp, "%d", &b->num_cols) || 
		fscanf(fp, "%d", &b->num_its)  ||
		fscanf(fp, "%d", &b->num_pairs )) != 1) {
			printError();
	}

	fclose(fp);
}	

void printBoardSpecs(Board *b) {
	printf("Num rows: %d\n", b->num_rows); 
	printf("Num cols: %d\n", b->num_cols);  
	printf("Num its:  %d\n", b->num_its); 
	printf("Pairs:    %d\n", b->num_pairs);
}


void printError() {
	printf("file data invalid");
}
