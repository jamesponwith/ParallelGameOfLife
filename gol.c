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
#include <pthread.h>

typedef struct {
	int num_rows;
	int num_cols;
	int num_its;
	int num_pairs;
	int size;
} BoardSpecs; 

typedef struct {
	BoardSpecs *bs;
	int *board;
	int verbose;
	int mytid;
	int start;
	int end;
	pthread_barrier_t *my_barrier;
} WorkerArgs;

void *Malloc(size_t size);
void printError(); 
void printBoardSpecs(BoardSpecs *b); 
int to1d(int row, int col, BoardSpecs *bs);
void printBoard(int *board, BoardSpecs *bs); 
void updateBoard(int *board, BoardSpecs *bs, int start, int end, pthread_barrier_t *pbt); 
//void *sim(int *board, BoardSpecs *bs, int verbose); 
void *sim(void* args); //TODO: how to proprly pass struct to void*args
int* initBoard(char *ascii_filename, BoardSpecs *b);
int numAlive(int *board, BoardSpecs *bs, int row, int col); 
void timeval_subtract(struct timeval *result, 
					struct timeval *end, struct timeval *start); 

void Filename(char *ascii_filename, char optarg);
/**
 * Prints out a reminder of how to run the program.
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: -c <%s> -v <verbose mode>\n", executable_name);
}

int main(int argc, char *argv[]) {
	char *ascii_filename = NULL;
	int num_threads = 4;
	int verbose = 0;
	int c = -1; 

	opterr = 0;
	while ((c = getopt(argc, argv, "c:vt:p")) != -1) {
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
			case 't':
				num_threads = strtol(optarg, NULL , 10);
				break;
			case 'p':
				//TODO:print out per thread board allocation
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
   	}

	pthread_t *tids = malloc(sizeof(pthread_t)*num_threads);
	WorkerArgs *thread_args = malloc(sizeof(WorkerArgs)*num_threads);
	BoardSpecs *bs = malloc(sizeof(BoardSpecs));


	int *board = initBoard(ascii_filename, bs);

	pthread_barrier_t my_barrier;
	if (pthread_barrier_init(&my_barrier, 0, num_threads)) {
		printf("pthread_barrier_init error\n");
		exit(1);
	}

	struct timeval start_time, curr_time, result;
	gettimeofday(&start_time, NULL); 	// get start time before starting game

	for (int i = 0; i < num_threads; i++) {
		thread_args[i].verbose = verbose;
		thread_args[i].bs = bs;
		thread_args[i].mytid = i;
		thread_args[i].board = board;
		if (i == 0) {
			thread_args[i].start = 0;
		}
		else {
			thread_args[i].start = thread_args[i - 1].end + 1;
		}

		int d = bs->num_rows / num_threads;
		int r = bs->num_rows % num_threads;
		if (i < r) {
			thread_args[i].end = thread_args[i].start + (d + 1) * bs->num_cols;
		}
		else {
			thread_args[i].end = thread_args[i].start + d * bs->num_cols;
		}
		pthread_create(&tids[i], NULL, sim, &thread_args[i]);
	}

	for (int i = 1; i < num_threads; i++) {
		pthread_join(tids[i], NULL);
	}


	gettimeofday(&curr_time, NULL); 	// check time after game
	timeval_subtract(&result, &curr_time, &start_time); // calculate time for program

	printf("Total time for %d iterations of %dx%d world is ", 
					bs->num_its, bs->num_cols, bs->num_rows);
	printf("%ld.%06ld\n", result.tv_sec, result.tv_usec);	

	free(thread_args);
	free(tids);
	free(board);
	free(bs);
	return 0;
}

/**
 * Simulates the game on terminal window
 * @param board The game board
 * @param bs The board's specificatoins
 */
void *sim(void *args) {
	WorkerArgs *w_args = (WorkerArgs*)args;	
	printf("Thread id: %d\n", w_args->mytid);
	if (w_args->mytid == 0) {
		if (w_args->verbose == 1) {
			printf("\nVerbose mode\n");
			//system("clear");
		}
	}
	for (int i = 0; i < w_args[w_args->mytid].bs->num_its; i++) {
		updateBoard(w_args->board, w_args->bs, w_args->start, w_args->end, w_args->my_barrier); 
		if (w_args->mytid == 0) {
			if (w_args->verbose == 1) {
				printf("Time step: %d\n", i);
				printBoard(w_args->board, w_args->bs);
				usleep(100000 * 2);
				if (!(i == w_args->bs->num_its)) {
					system("clear");
				}
			}
		}
	}
	return NULL;
}

/**
 * Updates the values in the board array
 * base off the rules of the game
 * @param board The board
 * @param bs The board's specs
 */
void updateBoard(int *board, BoardSpecs *bs, int start, int end, pthread_barrier_t *pbt) {
	int num_alive;
	int *tmp_board = (int*) calloc((bs->num_rows * bs->num_cols), sizeof(int)); 

	int start_r = start / bs->num_rows;
	int end_r = end / bs->num_rows;

	for (int i = start_r; i < end_r; i++) {
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
	pthread_barrier_wait(pbt);
	printf("after waiting\n");
	for (int i = start_r; i < end_r; i++) {
		for (int j = 0; j < bs->num_cols; j++) {
			board[to1d(i,j,bs)] = tmp_board[to1d(i,j,bs)];
		}
	}
	free(tmp_board);
}


/**
 * Gets the number of surrounding alive cells
 * @param board The game board
 * @param bs The board's specificaitons 
 * @param pos The position of the cell in the 1D array
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
 * @param fp The file object with board specifications
 * @param bs The board struct with its specs
 * @return the 2d board array, in a 1d int pointer array
 */
int* initBoard(char* ascii_filename, BoardSpecs *bs) {
	FILE *fp = fopen(ascii_filename, "r");
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
	for (int i = 0; i < bs->num_pairs; i++) {
		fscanf(fp, "%d %d", &col, &row); 
		int index = to1d(row, col, bs);
		board[index] = 1;
	}

	fclose(fp);
	return board;
}	

/**
 * Prints out the board
 * @param board The game board
 * @param bs The board's specs
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
 * @param bs the board's specs
 */
void printBoardSpecs(BoardSpecs *bs) {
	printf("Num rows: %d\n", bs->num_rows); 
	printf("Num cols: %d\n", bs->num_cols);  
	printf("Num its:  %d\n", bs->num_its); 
	printf("Pairs:    %d\n", bs->num_pairs);
}


/**
 * Converts 2d array coordinates to 1d array index
 * @param row The row index
 * @param col The col incex
 * @param bs The board specifications
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
 * @param start The start time
 * @param end The end time
 * @param result The resulting time difference
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

//wrapper for malloc calling perror and exit on error
void *Malloc(size_t size) {
	void *ret = malloc(size);
	if (!ret) {
		perror("malloc array");
		exit(1);
	}
	return ret;
}
