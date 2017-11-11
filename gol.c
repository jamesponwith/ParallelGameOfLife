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

// Forward function declarations
FILE* openFile(char *ascii_filename);

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: %s *rest of the args*", executable_name);
}

int main(int argc, char *argv[]) {
	char *ascii_filename = NULL;

	// Step 1: Parse command line args (I recommend using getopt again).
	// You need to support the "-c" and "-v" options for the basic requirements.
	// The advanced requirements require you to add "-l" and "-n" options.
	
	opterr = 0;
	int c = -1;

	while ((c == getopt(argc, argv, "")) != -1) {
		switch(c) {
			case 'c':
				ascii_filename = optarg;
				break;
			case 'v':
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
	
	// Step 3: Start your timer
	
	// Step 4: Simulate for the required number of steps.
	// Again, you should put this in its own function
	
	// Step 5: Stop your timer, calculate amount of time simulation ran for and
	// then print that out.
	return 0;
}
/**
	num rows
	num cols
	num iterations
	num of following coordinate pairs; set each (c, r) value to 1
	c r
	c r 
	...
 */

/**
 * Opens a file and returns a pointer to it.
 * If file does not exits, program exits(1).
 *
 * @param trace_file The file from which to read.
 *
 * @return fp The pointer to the file object created
 */

FILE* openFile(char *ascii_filename) {
	FILE *fp = fopen(ascii_filename, "r");
	if (fp == NULL) {
		printf("No such file\n");
		exit(1);
	}
	printf("trace file read\n");
	return fp;
}

