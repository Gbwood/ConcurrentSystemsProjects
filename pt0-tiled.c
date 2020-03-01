#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 8000000
#define STRING_SIZE 16

char char_array[ARRAY_SIZE][STRING_SIZE];
int char_counts[26];

char getRandomChar()
{
	int randNum = 0;
	char randChar = ' ';

	randNum = 26 * (rand() / (RAND_MAX + 1.0)); 	// pick number 0 < # < 25
	randNum = randNum + 97;				// scale to 'a'
	randChar = (char) randNum;

	// printf("%c", randChar);
	return randChar;
}

void init_arrays()
{
  int i, j, randNum; 
  char randChar;

  for ( i = 0; i < ARRAY_SIZE; i++) {
	for ( j = 0; j < STRING_SIZE; j++ ) {
		 char_array[i][j] = getRandomChar();
	}
  }

  for ( i = 0; i < 26; i++ ) {
  	char_counts[i] = 0;
  }
}

void count_array()
{
  char theChar;
  int i, j, charLoc;
  struct timeval t1, t2;
  double elapsedTime;
  int numSlots, myVersion = 1;

  gettimeofday(&t1, NULL);
  
  printf("DEBUG: starting loop on %s\n", getenv("HOSTNAME"));

  for ( i = 0; i < ARRAY_SIZE; i += 16) {
	for ( j = 0; j < STRING_SIZE; j += 16) {
		for ( k = i ; k < i+16; k++ ) {
			for (l = j ; l < j+16; k++) {
				theChar = char_array[k][l];
				charLoc = ((int) theChar) - 97;
				char_counts[charLoc]++;
			}
		}
	}
  }
  
  gettimeofday(&t2, NULL);


	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	printf("DATA, %d, %s, %f\n", myVersion, getenv("SLURM_NTASKS"),  elapsedTime);
}

void print_results()
{
  int i, total = 0;

  for ( i = 0; i < 26; i++ ) {
     total += char_counts[i];
     printf(" %c %d\n", (char) (i + 97), char_counts[i]);
  }
  printf("\nTotal characters:  %d\n", total);
}

main() {
	init_arrays();
	count_array();
	print_results();
}

