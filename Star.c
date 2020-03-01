#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include <mpi.h>

#define MAX_WORDS 50000
#define MAX_LINES 100000

double myclock();

typedef struct word_Info_s {
    int index;
    int count;
} word_Info;

int main(int argc, char ** argv) {
	int nwords, nlines, i, j, err, rc, size, rank, next, prev, startPos, endPos;
	int lineTag = 101;
	int wordTag = 50;
    int indexTag = 105;
	double nchars = 0;
    word_Info currentWord = {0,0};
	double tstart, ttotal;
	FILE *fd;
	static char wordList[MAX_WORDS][10];
	static char lineList[MAX_LINES][2001];
	static int countList[MAX_WORDS];
	memset(wordList, 0, sizeof(char) * MAX_WORDS * 10);
	memset(countList, 0, sizeof(int) * MAX_WORDS);
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

    /* create a type for struct car */
    const int nitems=2;
    int          blocklengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};
    MPI_Datatype mpi_word_info_type;
    MPI_Aint     offsets[2];

    offsets[0] = offsetof(word_Info, index);
    offsets[1] = offsetof(word_Info, count);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_word_info_type);
    MPI_Type_commit(&mpi_word_info_type);

	//If the root process, initialize the words and lines
	if (rank == 0) {
		//Initialize the keywords
		fd = fopen("/homes/dan/625/keywords.txt", "r");
		nwords = -1;
		do {
			err = fscanf(fd, "%[^\n]\n", wordList[++nwords]);	//read in dictionary
			countList[++nwords] = 0;
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
	
    int startingIndex = 0;
	//Send all the information to every process
	for (i = 1; i < size; ++i) {
		MPI_Send(wordList, MAX_WORDS * 10, MPI_CHAR, i, wordTag, MPI_COMM_WORLD);
		MPI_Send(lineList, MAX_LINES * 2001, MPI_CHAR, i, lineTag, MPI_COMM_WORLD);
   		MPI_Send(&startingIndex, 1, MPI_INT, i, indexTag, MPI_COMM_WORLD);
	}
	 
    

	//If not the root, process the words.
	}else{
        MPI_Recv(wordList, MAX_WORDS * 10, MPI_CHAR, 0, wordTag, MPI_COMM_WORLD, &status);
		MPI_Recv(lineList, MAX_LINES * 2001, MPI_CHAR, 0, lineTag, MPI_COMM_WORLD, &status);
    }
	
    

//struct index, word and count
	startPos = (rank-1) * (MAX_LINES / (size-1));
	endPos = startPos + (MAX_LINES / (size-1));

    if (rank != 0) {
        word_Info currentWordStruct;
        int wordIndex;
        while (1){
            //printf("Debug, process %d waiting on message. \n", rank);
            MPI_Recv(&wordIndex, 1, MPI_INT, 0, indexTag, MPI_COMM_WORLD, &status);
            //printf("Debug, process %d received message, lets start boys, wordIndex %d \n", rank, wordIndex);

            currentWordStruct.count = 0;
            currentWordStruct.index = wordIndex;

            for (i = startPos; i < endPos; i++) {           
                if (strstr(lineList[i], wordList[wordIndex]) != NULL) {
                     currentWordStruct.count++;
                }
            }
            
		    MPI_Send(&currentWordStruct, 1, mpi_word_info_type, 0, 0, MPI_COMM_WORLD);
                       // printf("Debug, process %d send result to rank 0, on word %d which is word %s \n", rank, currentWordStruct.count, wordList[wordIndex]);

            if(MAX_WORDS < ++currentWordStruct.index){
                //printf("DEBUG, process %d checked the last word %d. Sending back to process 0.\n", rank, currentWord);
                // MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                break;
            }
        }
    }
    
    if (rank == 0) {
        while(1) {
            for (int i = 1; i < size; i++) {
                    //printf("Debug, process %d waiting on message %d, lets start boys \n", rank, i);

                MPI_Recv(&currentWord, 1, mpi_word_info_type, i, 0, MPI_COMM_WORLD, &status);
                    //printf("Debug, process %d received message, lets start boys \n", rank);

                if (countList[currentWord.index] < 100) {
                    countList[currentWord.index] += currentWord.count;
                }
                if(MAX_WORDS < ++currentWord.index){
                   // printf("DEBUG, process %d checked the last word %d. Sending back to process 0.\n", rank, currentWord);
                    break;
                }
                else {
                    currentWord.index++;
                    MPI_Send(&currentWord.index, 1, MPI_INT, i, indexTag, MPI_COMM_WORLD);
                }
            }
            if(MAX_WORDS < ++currentWord.index){
                    break;
            }
        }
            
        //rank zero updates count
    }
	
	
	
	//print results
	if (rank == 0){
		// MPI_Recv(&currentWord, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		// printf("DEBUG Process %d ended. Printing results.\n", currentWord);
		for(i = 0; i < MAX_WORDS; i++){
            if (countList[i] > 0) {
                printf("%s: %d\n", wordList[i], countList[i]);
            }
			
		}
        ttotal = myclock() - tstart;
	    printf("DATA, %d, %s, %f\n", size, getenv("SLURM_NNODES"), ttotal);
	}
	
	//Get the final time and print information.
	
	
	MPI_Finalize();
}

    double myclock() {
        static time_t t_start = 0;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, & ts);
        if (t_start == 0) t_start = ts.tv_sec;

        return (double)(ts.tv_sec - t_start) + ts.tv_nsec * 1.0e-9;
    }
