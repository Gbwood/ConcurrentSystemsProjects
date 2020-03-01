#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

double myclock();
int nwords, maxwords = 50000;
int nlines, maxlines = 1000000;
char **word, **line;
int size;

void *check_array(void* rank) {
   int myRank = *((int *)rank);

   int startingPosition = ((long)myRank) * (nwords/size)




}


int main()
{
   int i, k, n, err, rc, *count, nthreads = 24;
   double nchars = 0;
   double tstart, ttotal;
   FILE *fd;
   int rank;
  
   //intitialize MPI
   rc = MPI_Init(&argc, &argv);
	if(rc != MPI_SUCCESS)
	{
		printf("Error starting MPI program. Terminating. \n");
	}
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);




// Malloc space for the word list and lines
   word = (char **) malloc( maxwords * sizeof( char * ) );
  // count = (int *) malloc( maxwords * sizeof( int ) );
   for( i = 0; i < maxwords; i++ ) {
      word[i] = malloc( 10 );
      //count[i] = 0;
   }

   line = (char **) malloc( maxlines * sizeof( char * ) );
   for( i = 0; i < maxlines; i++ ) {
      line[i] = malloc( 2001 );
   }


// Read in the dictionary words
if (rank == 0) {
   fd = fopen("/homes/dan/625/keywords.txt", "r");
   nwords = -1;
   do {
      err = fscanf( fd, "%[^\n]\n", word[++nwords] );
   } while( err != EOF && nwords < maxwords );
   fclose( fd );

   printf( "Read in %d words\n", nwords);


// Read in the lines from the data file

   fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
   nlines = -1;
   do {
      err = fscanf( fd, "%[^\n]\n", line[++nlines] );
      if( line[nlines] != NULL ) nchars += (double) strlen( line[nlines] );
   } while( err != EOF && nlines < maxlines);
   fclose( fd );

   printf( "Read in %d lines averaging %.0lf chars/line\n", nlines, nchars / nlines);

   tstart = myclock(); // Set the zero time
	tstart = myclock(); // Start the clock

}
   

   //need to broadcast all words to all nodes
   //need to only broadcast specific lines to specific nodes?? don't know how to do that yet

   



// Loop over the word list


//   omp_set_num_threads( nthreads );

#pragma omp parallel for schedule( dynamic ) private(i,k)
   for( i = 0; i < nwords; i++ ) {

      for( k = 0; k < nlines; k++ ) {
         if( strstr( line[k], word[i] ) != NULL ) count[i]++;
      }

   }

   ttotal = myclock() - tstart;
   printf( "The run on %d cores took %lf seconds for %d words\n",
           nthreads, ttotal, nwords);

// Dump out the word counts

   fd = fopen( "wiki.out", "w" );
   for( i = 0; i < nwords; i++ ) {
      fprintf( fd, "%d %s %d\n", i, word[i], count[i] );
   }
   fprintf( fd, "The run on %d cores took %lf seconds for %d words\n",
           nthreads, ttotal, nwords);
   fclose( fd );

}

double myclock() {
   static time_t t_start = 0;  // Save and subtract off each time

   struct timespec ts;
   clock_gettime(CLOCK_REALTIME, &ts);
   if( t_start == 0 ) t_start = ts.tv_sec;

   return (double) (ts.tv_sec - t_start) + ts.tv_nsec * 1.0e-9;
}
