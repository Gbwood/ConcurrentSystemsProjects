#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include <mpi.h>

#define MAX_WORDS 50000
#define MAX_LINES 100000

double myclock();

int main(int argc, char ** argv) {
	int nwords, nlines, i, j, err, rc, size, rank, startPos, endPos;
	int lineTag = 101;
	int wordTag = 50;
	double nchars = 0;
	double tstart, ttotal;
	FILE *fd;
	static char wordList[MAX_WORDS/100][100][10];
	char temp [100][10];
	static char lineList[MAX_LINES][2001];
	static int wordCount[MAX_WORDS];
	static int batch[1];
	memset(wordList, 0, sizeof(char) * MAX_WORDS * 10);
	memset(wordCount, 0, sizeof(int) * MAX_LINES);
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
	batch[0] = rank;

	//If the root process, initialize the words and lines
	if (rank == 0) {
		//Initialize the keywords
		fd = fopen("/homes/dan/625/keywords.txt", "r");
		nwords = -1;
		i = 0;
		do {
			if((nwords+1)%100 == 0)
				i++;
			err = fscanf(fd, "%[^\n]\n", wordList[i][++nwords]);	//read in dictionary 
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
		
		//Send all lists to all processes
		for (i = 1; i < size; ++i) {	
			memcpy(temp, wordList[i-1], sizeof(temp));
			batch[0]++;
			MPI_Send(temp, 1000, MPI_CHAR, i, wordTag, MPI_COMM_WORLD);	
			MPI_Send(lineList, MAX_LINES * 2001, MPI_CHAR, i, lineTag, MPI_COMM_WORLD);
			MPI_Send(wordCount, MAX_WORDS, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
		
		
		//Get the final time and print information.
		ttotal = myclock() - tstart;
		printf("DATA, %d, %s, %f\n", size, getenv("SLURM_NNODES"), ttotal);
	} 
	//If not the root, process the words.
	
	
	else {
		MPI_Recv(temp, 1000, MPI_CHAR, 0, wordTag, MPI_COMM_WORLD, &status);
		MPI_Recv(lineList, MAX_LINES * 2001, MPI_CHAR, 0, lineTag, MPI_COMM_WORLD, &status);
		MPI_Recv(wordCount, MAX_WORDS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}
		
	while(1){
		if(rank == 0){
			MPI_Recv(wordCount, MAX_WORDS, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			if(batch[0] < MAX_WORDS/100){
				memcpy(temp, wordList[i-1], sizeof(temp));
				batch[0]++;
				MPI_Send(temp, 1000, MPI_CHAR, status.MPI_SOURCE, wordTag, MPI_COMM_WORLD);
				MPI_Send(batch, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			}
			else break;
		}
		else{
			for(i = 0; i < 100; ++i){
				for (j = 0; j < MAX_LINES; j++) {
					if (strstr(lineList[j], wordList[batch[0]][i]) != NULL)
						wordCount[i]++;
				}
			}
			MPI_Send(wordCount, MAX_WORDS, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(temp, 1000, MPI_CHAR, 0, wordTag, MPI_COMM_WORLD, &status);
			MPI_Recv(batch, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			if(batch[0] >= MAX_WORDS/100) break;
		}
	}
	
	
	if(rank == 0){
		for(i = 0; i < nwords; i++){
			if(wordCount[i] > 0)
			printf("WORDS, %s: %d\n", wordList[i], j+1);
		}
		ttotal = myclock()-tstart;
		printf("DATA, size, NNodes, Time\n");
		printf("DATA, %d, %s, %f\n", size, getenv("SLURM_NNODES"), ttotal);
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