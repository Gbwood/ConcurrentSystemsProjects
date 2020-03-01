#include <stdio.h>
#include <omp.h>
#include <sys/time.h>
#include <stdlib.h>

#define NUM_THREADS 4

double total = 0.0;


void *calc(int thread) {
	const int NUM_ITER = 100000000;
	int i;
	double sum = 0.0, x = 0.0;
	double st = 2.0/((double) NUM_ITER);
	int startPos, endPos;
	
	
	
    #pragma omp private(threads,startPos,endPos, i, sum, st)
	{
		startPos = thread * (NUM_ITER / NUM_THREADS);
		endPos = startPos + (NUM_ITER / NUM_THREADS);
		
		printf("thread = %d startPos = %d endPos = %d \n", thread, startPos, endPos);

		for(i = startPos; i < endPos; i++)
		{
		  x = (i + 0.25)*st;
		  sum += 4.0/(x*x+1);
		}
	
	
		#pragma omp critical
		{
		total += sum;
		}
	}
}

main() {
	int myVersion = 1;
	struct timeval t1, t2;
    double elapsedTime;
	
	omp_set_num_threads(NUM_THREADS);
	
	gettimeofday(&t1, NULL);
	
	#pragma omp parallel 
	{
		calc(omp_get_thread_num());
	}
	printf("\n OpenMP Total:  %f\n", total);
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	printf("DATA, %d, %s, %f\n", myVersion, getenv("SLURM_NTASKS"),  elapsedTime);
}
