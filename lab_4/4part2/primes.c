#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "mythreads.h"


int primetest(int v);
void workerthread();

int number, currentNumber, nOfThreads;
sem_t *assignNumberSem, *availableNumberSem;

int main(int argc, char* argv[])
{
	int threadIndex, i;

	nOfThreads = atoi(argv[1]); //Create the number of threads specified
	thr_t* thread[nOfThreads];

	//mythreads_sem_initialization();
	assignNumberSem = malloc(sizeof(sem_t)); //This makes sure that only one thread at a time is assigned a number to check
	mythreads_sem_init(assignNumberSem, 0);
	availableNumberSem = malloc(sizeof(sem_t)); //This makes sure that only one number at a time is assigned to a thread
	mythreads_sem_init(availableNumberSem, 0);


	printf("Waiting for the threads to be created...\n");

	mythreads_init();
	//Thread creation
	for(threadIndex=0; threadIndex<nOfThreads; threadIndex++)
	{
		
		thread[threadIndex] = malloc(sizeof(thr_t));
		mythreads_create(thread[threadIndex], &workerthread, NULL);
	} 
	printf("All %d threads created\nAssigning numbers...\n", nOfThreads);

	//Numbers assignment
	for(i=2; i<=argc-1; i++)
	{	
		number = atoi(argv[i]);
		mythreads_sem_up(availableNumberSem); //Notify the workers that there is a new number to check
		mythreads_sem_down(assignNumberSem); //Wait for a thread to get assigned to checking this number	
	}
 
 	number = -133223; //Terminating value


	printf("\nNotifying workers to terminate...\n");
	for(i=0; i<nOfThreads; i++)
	{
		mythreads_sem_up(availableNumberSem);
	}

	for(i=nOfThreads-1; i>-1; i--)
	{
		mythreads_join(thread[i]);
	}

	printf("\nDestroying semaphores...\n");
	mythreads_sem_destroy(assignNumberSem);
	mythreads_sem_destroy(availableNumberSem);

	printf("\nDestroying threads...\n");
	for(i=0; i<nOfThreads; i++)
	{
		mythreads_destroy(thread[i]);
	}

	printf("\nTerminating...\n");
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

void workerthread() 
{
	int isprime, numberInThisThread;

	while(1)
	{	
 		mythreads_sem_down(availableNumberSem); //Wait for a new number that needs to be checked
 
		if(number == currentNumber) //Trying to check a number already checked
		{
			continue;
		}

		if(number == -133223) //No numbers left
		{
			break;
		}
		
		currentNumber = number;
		numberInThisThread = currentNumber;

		printf("Thread %d, checking number %d\n", curThread, numberInThisThread);

		mythreads_sem_up(assignNumberSem); //A number can be assigned to another thread		
		isprime = primetest(numberInThisThread); //The prime check is executed outside the CS
		if (isprime == 1)
		{
			printf("[Number %d is prime!]\n",numberInThisThread);
		}
		mythreads_yield();	
	}
}