//gcc -Wall main.c -o main -lpthread mypipe.c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "mypipe.h"

int main(int argc, char *argv[])
{
	pthread_t t1, t2;
	int check1, check2;
	char c='A';
	int size = 64;

	pipe_init(size);
	
	check1 = pthread_create(&t1, NULL, pipe_write, &c);
	if(check1)
	{
		fprintf(stderr,"Error - pthreat_create() return code: %d\n",check1);
		exit(EXIT_FAILURE);
	}

	check2 = pthread_create(&t2, NULL, pipe_read, &c);
	if(check2)
	{
		fprintf(stderr,"Error - pthreat_create() return code: %d\n",check2);
		exit(EXIT_FAILURE);
	}
	
	sleep(5);
	return 0;
}