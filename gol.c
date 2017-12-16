/**
 * File: gol.c
 * Author: Patrick Hall 
 *		   James Ponwith
 * This program simulates Conway's game of life using multiple threads.
 * The number of threads is specified by the -t argument; it is 4 by default.
 * The starting board specifications are passed through the command line
 * argument.
 * The program will run until the specified number of iterations has passed.
 * Ex: ./gol -c tests/test.txt -v -t 10 -p
 *
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
	int size;
	int num_its;
	int num_rows;
	int num_cols;
	int num_pairs;
} BoardSpecs; 

typedef struct {
	int end;
	int mytid;
	int start;
	int *board;
	int verbose;
	BoardSpecs *bs;
	pthread_barrier_t *my_barrier;
} WorkerArgs;

void printError(); 
void *sim(void* args); 
void printBoardSpecs(BoardSpecs *b); 
int to1d(int row, int col, BoardSpecs *bs);
void printBoard(int *board, BoardSpecs *bs); 
int* initBoard(char *ascii_filename, BoardSpecs *b);
int numAlive(int *board, BoardSpecs *bs, int row, int col); 
void printThreadStats(WorkerArgs *w_args, int num_threads, int num_rows);
void updateBoard(int *board, BoardSpecs *bs, int start, int end, pthread_barrier_t *pbt); 
void timeval_subtract(struct timeval *result, 
		struct timeval *end, struct timeval *start); 
void createThreads(WorkerArgs *thread_args, int *board, BoardSpecs* bs, 
		int verbose, pthread_t *tids, int num_threads, pthread_barrier_t my_barrier); 

/**
 * Prints out a reminder of how to run the program.
 * @param executable_name String containing the name of the executable
 */
void usage(char *executable_name) {
	printf("Usage: -c <%s> -v <verbose mode>\n", executable_name);
}

int main(int argc, char *argv[]) {
	int p = 0;
	int c = -1; 
	int verbose = 0;
	int num_threads = 4;
	char *ascii_filename = NULL;

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
				p = 1;	
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
	}
	// Allocate memory for threads and board
	pthread_t *tids = malloc(num_threads *sizeof(pthread_t));
	WorkerArgs *thread_args = malloc(num_threads *sizeof(WorkerArgs));
	BoardSpecs *bs = malloc(sizeof(BoardSpecs));

	int *board = initBoard(ascii_filename, bs);

	if (num_threads > bs->num_rows || num_threads <= 0) {
		printf("ERROR: invalid number of threads\n");
		exit(1);
	}

	pthread_barrier_t my_barrier;
	if (pthread_barrier_init(&my_barrier, NULL, num_threads) != 0) {
		perror("pthread_barrier_init error\n");
		exit(1);
	}

	struct timeval start_time, curr_time, result;
	gettimeofday(&start_time, NULL); 	
	
	/* Creates worker threads, runs game in parallel, and collectively updates
	 * the board.
	*/
	createThreads(thread_args, board, bs, verbose, tids, num_threads, my_barrier);

	// Join and handle worker threads
	for (int i = 0; i < num_threads; i++) {
		pthread_join(tids[i], NULL);
	}
	
	// Destroy barrier
	pthread_barrier_destroy(&my_barrier);

	gettimeofday(&curr_time, NULL); 
	timeval_subtract(&result, &curr_time, &start_time); 

	if(p == 1) {
		printThreadStats(thread_args, num_threads, bs->num_rows);
	}

	printf("Total time for %d iterations of %dx%d world is ", 
			bs->num_its, bs->num_cols, bs->num_rows);
	printf("%ld.%06ld\n", result.tv_sec, result.tv_usec);	

	// Frees all dynamically allocated memory
	free(bs);
	free(tids);
	free(board);
	free(thread_args);
	return 0;
}

/**
 * Populate thread args, creates threads, calls sim on each thread
 *
 * @param my_barrier - pthread Barrier
 * @param *tids - Array of worker threads
 * @param *thread_args - a struct containing each threads arguements 
 * @param *board - pointer to the array which represents the game board
 * @param verbose - int value determining if verbose mode should be enabled
 * @param num_threads - the number of worker threads to be created (4 default)
 */
void createThreads(WorkerArgs *thread_args, int *board, BoardSpecs* bs, int verbose, pthread_t *tids, int num_threads, pthread_barrier_t my_barrier) {
	for (int i = 0; i < num_threads; i++) {
		thread_args[i].bs = bs;
		thread_args[i].mytid = i;
		thread_args[i].board = board;
		thread_args[i].verbose = verbose;
		thread_args[i].my_barrier = &my_barrier;
		// Provide the first thread with starting row 0
		if (i == 0) {
			thread_args[i].start = 0;
		}
		// Set start row to proceeding threads
		else {
			thread_args[i].start = thread_args[i - 1].end + 1;
		}

		// Get the number of rows for each thread to do
		int d = bs->num_rows / num_threads;
		// Get the remainder for case the threads and row do not divide evenly
		int r = bs->num_rows % num_threads;
		
		// If thread id is less than the remainder do an additional row  
		if (i < r) {
			thread_args[i].end = thread_args[i].start + (d + 1) * bs->num_cols;
		}
		// Handle the threads that are accepting the even number of divisible
		else {
			thread_args[i].end = thread_args[i].start + d * bs->num_cols;
		}
		// Spawn threads
		pthread_create(&tids[i], NULL, sim, &thread_args[i]);
	}
}

/**
 * Print the statistics for each thread
 * @param w_args The arguments for worker threads
 * @num_threads Number of worker threads
 * @num_rows Number of rows in the game board
 */
void printThreadStats(WorkerArgs *w_args, int num_threads, int num_rows) {
	for(int i = 0; i < num_threads; i++) {
		int start_row = w_args[i].start / num_rows;
		int end_row = (w_args[i].end / num_rows) - 1;
		fprintf(stdout, "tid %d:\trows:\t %d:%d\t(%d)\n", 
				w_args[i].mytid, start_row, end_row, (end_row - start_row) + 1);
		fflush(stdout);
	}
}

/**
 * Simulates the game on terminal window
 * @param board The game board
 * @param bs The board's specificatoins
 */
void *sim(void *args) {
	WorkerArgs *w_args = (WorkerArgs*)args;	
	if (w_args->mytid == 0) {
		if (w_args->verbose == 1) {
			printf("\nVerbose mode\n");
			system("clear");
		}
	}
	int its = w_args->bs->num_its;
	for (int i = 0; i < its; i++) {
		updateBoard(w_args->board, w_args->bs, w_args->start, w_args->end, w_args->my_barrier); 
		if (w_args->mytid == 0) {
			if (w_args->verbose == 1) {
				printf("Time step: %d\n", i);
				printBoard(w_args->board, w_args->bs);
				usleep(100000 * 2);
				if (!(i == its - 1)) {
					system("clear");
				}
			}
		}
		//pthread_barrier_wait(w_args->my_barrier);
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
	// Pause and wait for all threads to gather
	pthread_barrier_wait(pbt);
	for (int i = start_r; i < end_r; i++) {
		for (int j = 0; j < bs->num_cols; j++) {
			board[to1d(i,j,bs)] = tmp_board[to1d(i,j,bs)];
		}
	}
	// Pause and wait again
	pthread_barrier_wait(pbt);
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
			printf("˙ "); 
		}
		else { 
			printf(" "); 
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
 * @param start The start time
 * @param end The end time
 * @param result The resulting time difference
 */
void timeval_subtract(struct timeval *result, 
		struct timeval *end, struct timeval *start) 
{
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
	result->tv_sec = end->tv_sec - start->tv_sec;
	result->tv_usec = end->tv_usec - start->tv_usec;
}
