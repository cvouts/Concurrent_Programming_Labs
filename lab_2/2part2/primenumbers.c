// ./primenumbers nOfThreads [the numbers that we want to check]

#define _GNU_SOURCE
#define SEM1_KEY "../2part1/ftok.txt"
#define SEM2_KEY "../2part1/ftok2.txt"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "../2part1/mysem.c"

int primetest(int v);
void* workerthread(void* ptr);

int number, assignNumberSemID,availableNumberSemID, currentNumber, nOfThreads;

int main(int argc, char* argv[])
{
	int threadIndex, threadCheck, i;
	key_t key1, key2;

	nOfThreads = atoi(argv[1]); //Create the number of threads specified
	pthread_t thread[nOfThreads];

	if ((key1 = ftok (SEM1_KEY, 'a')) == -1) 
	{
		perror ("ftok 1"); exit (1);
	}
	if ((key2 = ftok (SEM2_KEY, 'a')) == -1) 
	{
		perror ("ftok 2"); exit (1);
	}

	assignNumberSemID=mysem_create(key1); //This makes sure that only one thread at a time is assigned a number to check
	availableNumberSemID=mysem_create(key2); //This makes sure that only one number at a time is assigned to a thread

	number = 'a'; //Initialize it as not a number

	printf("Waiting for the threads to be created...\n");

	//Thread creation
	for(threadIndex=0; threadIndex<nOfThreads; threadIndex++)
	{
		threadCheck= pthread_create(&thread[threadIndex], NULL, workerthread, (void*) (intptr_t) threadIndex);
		if(threadCheck)
		{ 
	    	fprintf(stderr,"Error - pthread_create() return code: %d\n", threadCheck);
        	exit(EXIT_FAILURE);
		}
	} 
	printf("All %d threads created\n", nOfThreads);

	mysem_down(assignNumberSemID); //Initialize the semaphore value as 0

	//Numbers assignment
	for(i=2; i<=argc-1; i++)
	{	
		number = atoi(argv[i]);
		printf("Assigning number %d\n", number);
		mysem_up(availableNumberSemID); //Notify the workers that there is a new number to check
		mysem_down(assignNumberSemID); //Wait for a thread to get assigned to checking this number	
	}
 
 	number = 'x'; //Again, a value that is not a number

	printf("Notifying workers to terminate...\n");
	for(i=0; i<nOfThreads; i++)
	{
		mysem_up(availableNumberSemID);
	}

	for(threadIndex=0; threadIndex<=nOfThreads-1; threadIndex++)
	{
		pthread_join(thread[threadIndex], NULL);
	} 

	printf("Terminating...\n");
	mysem_destroy(assignNumberSemID);
	mysem_destroy(availableNumberSemID);
	return 0;
}

int primetest(int v) //If not prime return 0, if prime return 1
{
	int i;
	
	for(i=2; i<v; i++)
	{
		if(v<=2){
			return 1;
		}
		if(v%i == 0)
		{	
			return 0;
		}
	}
	return 1;
}

void* workerthread(void* ptr) 
{
	int isprime, numberInThisThread;
	int i=(int)(long long int) ptr;

	while(1)
	{	
 		mysem_down(availableNumberSemID); //Wait for a new number that needs to be checked
 
		if(number == 'a') //There are no numbers to check yet
		{
			continue; //Go back to mysem_down(availableNumberSemID) to get locked
		}

		if(number == currentNumber) //Trying to check a number already checked
		{
			mysem_up(availableNumberSemID);
			continue;
		}

		if(number == 'x') //No numbers left
		{
			printf("Thread %d can now terminate\n", i+1);
			break;
		}
		
		currentNumber = number;
		numberInThisThread = currentNumber;

		printf("Thread %d, checking number %d\n", i+1, numberInThisThread);

		mysem_up(assignNumberSemID); //A number can be assigned to another thread		

		isprime = primetest(numberInThisThread); //The prime check is executed outside the CS
		if (isprime == 1){
			printf("[Number %d is prime!]\n",numberInThisThread);
		}	
	}
	pthread_yield();
	return NULL;
}

/* There is chance that the same worker thread will try to read a new number between the unlocking of main at line 64 and setting a new
number at line 61. This worker thread goes from line 134 back up to line 117 and we are forced to up a semaphore that already had a value of 1.
A pthread_yield() is used for this, at line 141 */
