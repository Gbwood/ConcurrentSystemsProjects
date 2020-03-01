#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

#define maxwords 50000
#define maxlines 100000


double myclock();
int nwords, 
int nlines, 
int NUM_THREADS;
int *count;

char **word, **line;
int tempCount;


void *wiki_count(void *rank) {
	
	int myID =  *((int*) rank);
	int i, k;
	int *tempCount = (int *) malloc( maxwords * sizeof( int ) );
	
	
	int startPos = ((long) myID) * (nlines / NUM_THREADS);
	int endPos = startPos + (nlines / NUM_THREADS);

	printf("myID = %d startPos = %d endPos = %d \n", myID, startPos, endPos); fflush(stdout);

	// init local count array
	for ( i = 0; i < nwords; i++ ) {
		tempCount[i] = count[i];
	}
	
	#pragma omp parallel for schedule( dynamic ) private(i,k)
    for( i = 0; i < nwords; i++ ) {
       for ( k = startPos; k < endPos; k++ ) {
          if ( strstr( line[k], word[i] ) != NULL ) tempCount[i]++;
       }
   }

   ttotal = myclock() - tstart;
   printf( "The run on %d cores took %lf seconds for %d words\n", NUM_THREADS, ttotal, nwords);


	
}

int main(int argc, char* argv[])
{
   int nwords, nlines, rc;
   int i, k, n, err;
   int rc;
   double nchars = 0;
   double tstart, ttotal;
   
   FILE *fd;
   
   //int numtasks = (int) strtol(getenv("SLURM_NPROCS"), (char **)NULL, 10);
   int rank, numtasks;
	

    MPI_Status Status;

	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS) {
	  printf ("Error starting MPI program. Terminating.\n");
          MPI_Abort(MPI_COMM_WORLD, rc);
        }

    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	
	printf("size = %d rank = %d\n", numtasks, rank);
	fflush(stdout);

	// Malloc space for the word list and lines
	static char word[maxwords][10];
	static char line[maxlines][2001];
	memset(wordList, 0, sizeof(char) * maxwords * 10);
    memset(lineList, 0, sizeof(char) * maxlines * 2001);
	
	word = (char **) malloc( maxwords * sizeof( char * ) );
	   count = (int *) malloc( maxwords * sizeof( int ) );
	   for( i = 0; i < maxwords; i++ ) {
		  word[i] = malloc( 10 );
		  count[i] = 0;
	   }

	   line = (char **) malloc( maxlines * sizeof( char * ) );
	   for( i = 0; i < maxlines; i++ ) {
		  line[i] = malloc( 2001 );
	   }

	if (rank = 0) {
	   


		// Read in the dictionary words
	
	   fd = fopen( "/homes/dan/625/keywords.txt", "r" );
	   nwords = -1;
	   do {
		  err = fscanf( fd, "%[^\n]\n", word[++nwords] );
	   } while( err != EOF && nwords < maxwords );
	   fclose( fd );

	   printf( "Read in %d words\n", nwords);


		// Read in the lines from the data file
		
	   fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
	   nlines = -1;
	   do {
		  err = fscanf( fd, "%[^\n]\n", line[++nlines] );
		  if( line[nlines] != NULL ) nchars += (double) strlen( line[nlines] );
	   } while( err != EOF && nlines < maxlines);
	   fclose( fd );

	   printf( "Read in %d lines averaging %.0lf chars/line\n", nlines, nchars / nlines);
	}

	// Loop over the word list

	MPI_Bcast(word, maxwords, MPI_CHAR, 0, MPI_COMM_WORLD); //broadcast message to other threads //other threads blocked until they receive message //this will be done by rank == 0 and received by all otherr ranks
	MPI_Bcast(line, maxlines, MPI_CHAR, 0, MPI_COMM_WORLD);

    tstart = myclock();  // Set the zero time
    tstart = myclock();  // Start the clock

	//omp_set_num_threads( nthreads );
	
	wiki_count(&rank);

   ttotal = myclock() - tstart;
   printf( "The run on %d cores took %lf seconds for %d words\n", NUM_THREADS, ttotal, nwords);

   


	MPI_Reduce(tempCount, count, maxwords, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); //DO IT FOR ME


	if (rank == 0) {
		// Dump out the word counts

	   fd = fopen( "wiki.out", "w" );
	   for( i = 0; i < nwords; i++ ) {
		  fprintf( fd, "%d %s %d\n", i, word[i], count[i] );
	   }
	   fprintf( fd, "The run on %d cores took %lf seconds for %d words\n",
			   NUM_THREADS, ttotal, nwords);
	   fclose( fd );
		
	}
	
	MPI_Finalize();
	return 0;
}

double myclock() {
   static time_t t_start = 0;  // Save and subtract off each time

   struct timespec ts;
   clock_gettime(CLOCK_REALTIME, &ts);
   if( t_start == 0 ) t_start = ts.tv_sec;

   return (double) (ts.tv_sec - t_start) + ts.tv_nsec * 1.0e-9;
}
