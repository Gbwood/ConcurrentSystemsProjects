#include <stdio.h>
#include <stdlib.h>


main() {
	const int NUM_ITER = 100000000;
	int i;
	int myVersion = 1;
	struct timeval t1, t2;
    double elapsedTime;
	double sum = 0.0, x = 0.0;
	double st = 2.0/((double) NUM_ITER);
	
	gettimeofday(&t1, NULL);

	for(i = 0; i < NUM_ITER; i++)
	{
	  x = (i + 0.25)*st;
	  sum += 4.0/(x*x+1);
	}
	
	printf("\n Default Total:  %f\n", sum);
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	printf("DATA, %d, %s, %f\n", myVersion, getenv("SLURM_NTASKS"),  elapsedTime);
}

