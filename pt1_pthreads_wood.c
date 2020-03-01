#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>

#define NUM_THREADS 4

double total = 0.0;
pthread_mutex_t mutexsum;

void *calc(void *thread) {
	const int NUM_ITER = 100000000;
	int i;
	double sum = 0.0, x = 0.0;
	double st = 2.0/((double) NUM_ITER);
	int startPos, endPos;
	
	
	

		startPos = ((int)thread) * (NUM_ITER / NUM_THREADS);
		endPos = startPos + (NUM_ITER / NUM_THREADS);
		
		printf("thread = %d startPos = %d endPos = %d \n",(int) thread, startPos, endPos);

		for(i = startPos; i < endPos; i++)
		{
		  x = (i + 0.25)*st;
		  sum += 4.0/(x*x+1);
		}
	
	
		pthread_mutex_lock (&mutexsum);
		total += sum;
		pthread_mutex_unlock (&mutexsum);

		pthread_exit(NULL);
	
}

main() {
	int i, rc;
	int myVersion = 1;
	struct timeval t1, t2;
    double elapsedTime;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;
	
	gettimeofday(&t1, NULL);
	
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	
	for (i = 0; i < NUM_THREADS; i++ ) {
	      rc = pthread_create(&threads[i], &attr, calc, (void *)i);
	      if (rc) {
	        printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	      }
	}
	
	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);
	for(i=0; i<NUM_THREADS; i++) {
	     rc = pthread_join(threads[i], &status);
	     if (rc) {
		   printf("ERROR; return code from pthread_join() is %d\n", rc);
		   exit(-1);
	     }
	}
	
	printf("\n PThread Total:  %f\n", total);
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	printf("DATA, %d, %s, %f\n", myVersion, getenv("SLURM_NTASKS"),  elapsedTime);
	pthread_mutex_destroy(&mutexsum);
	printf("Main: program completed. Exiting.\n");
	pthread_exit(NULL);
	
	
}
