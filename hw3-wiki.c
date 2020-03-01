#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include <mpi.h>

#define MAX_WORDS 50000
#define MAX_LINE 100000

double myclock();

int main(int argc, char ** argv) {
	int nwords, nlines, i, j, err, rc, size, rank;
	double nchars = 0;
	double tstart, ttotal;
	FILE *fd;
	static char wordList[MAX_WORDS][10];
	static char lineList[MAX_LINES][2001];
	memset(wordList, 0, sizeof(char) * MAX_WORDS * 10);
	memset(lineList, 0, sizeof(char) * MAX_LINES * 2001);
	MPI_Status status;
  
	//Initialize the MPI
	rc = MPI_Init(&argc, &argv);
	if(rc != MPI_SUCCESS)
	{
		printf("Error starting MPI program. Terminating. \n");
	}
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//If the root process, initialize the words and lines
	if (rank == 0) {
		//Initialize the keywords
		fd = fopen("/homes/dan/625/keywords.txt", "r");
		nwords = -1;
		do {
			err = fscanf(fd, "%[^\n]\n", wordList[++nwords]);	//read in dictionary 
		} while (err != EOF && nwords < MAX_WORDS);
		fclose(fd);

	printf("Read in %d words from dictionary\n", nwords);
	
	//Initialize the Wikipedia lines
	fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
	nlines = -1;
	do {
		err = fscanf(fd, "%2000s\n", lineList[++nlines]);	//read in wiki dump
		if (lineList[nlines] != NULL) nchars += (double) strlen(lineList[nlines]);
	} while (err != EOF && nlines < MAX_LINES);
	fclose(fd);

	printf("Read in %d lines from data file averaging %.0lf chars/line\n", nlines, nchars / nlines);
	
	//Start the timer
	tstart = myclock(); // Set the zero time
	tstart = myclock(); // Start the clock
	
	//Send the lists to all processes
	for (i = 1; i < size; ++i) {
		MPI_Send(wordList, MAX_WORDS * 10, MPI_CHAR, i, 0, MPI_COMM_WORLD);
		MPI_Send(lineList, MAX_LINES * 2001, MPI_CHAR, i, 0, MPI_COMM_WORLD);
	}
	
	//Processes the words in the root
	for (i = 0; i < MAX_WORDS; ++i) {
		int from = i % (size - 1) + 1;
		MPI_Recv(&j, 1, MPI_INT, from, 0, MPI_COMM_WORLD, &status);
		if (j == -1)
			continue;
		printf("%s: %d", wordList[i], j);
		while (1) {
			MPI_Recv(&j, 1, MPI_INT, from, 0, MPI_COMM_WORLD, &status);
			if (j == -1) {
				printf("\n");
				break;
			}
			printf(", %d", j);
		}
	}
	
	//Get the final time and print information.
	ttotal = myclock() - tstart;
	printf("DATA, %d, %s, %f\n", size, getenv("SLURM_NNODES"), ttotal);

	//If not the root, process the words.
	} else {
		MPI_Recv(wordList, MAX_WORDS * 10, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(lineList, MAX_LINES * 2001, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

		for (i = 0; i < MAX_WORDS; i += (size - 1)) {
			for (j = 0; j < MAX_LINES; j++) {
				if (strstr(lineList[j], wordList[i]) != NULL)
					MPI_Send(&j, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			}
				j = -1;
				MPI_Send(&j, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
			MPI_Send(&j, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
}

double myclock() {
	static time_t t_start = 0;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, & ts);
	if (t_start == 0) t_start = ts.tv_sec;

	return (double)(ts.tv_sec - t_start) + ts.tv_nsec * 1.0e-9;
}