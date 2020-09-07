// ./primes nOfThreads [the numbers that we want to check]

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int primetest(int v);
void* workerthread(void* ptr);

pthread_cond_t cond;
pthread_mutex_t mutex;
int number,nOfThreads,currentNumber;
int JobIsDone;

int main(int argc,char* argv[])
{
	int threadIndex, threadCheck,mutex_check,cond_check, i;

	nOfThreads = atoi(argv[1]); //Create the number of threads specified
	pthread_t thread[nOfThreads];
	JobIsDone=0; //Job is not done

	//Mutex creation
	mutex_check=pthread_mutex_init(&mutex, NULL);
	if (mutex_check) 
    { 
        fprintf(stderr,"Error - pthread_mutex_init() return code: %d\n", mutex_check);
        exit(EXIT_FAILURE);
    } 

    //Condition variable creation
    cond_check=pthread_cond_init(&cond, NULL);
    if (cond_check) 
    { 
        fprintf(stderr,"Error - pthread_cond_init() return code: %d\n", cond_check);
        exit(EXIT_FAILURE);
    } 

	printf("Waiting for the threads to be created...\n");
	pthread_mutex_lock(&mutex);

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
	pthread_mutex_unlock(&mutex);

	for(i=2; i<=argc-1; i++)
	{	
		number = atoi(argv[i]);
		printf("\nAssigning number %d\n", number);
		pthread_cond_wait(&cond,&mutex); //Wait for a thread to get assigned to checking this number
	}

	JobIsDone=1; //Inform threads that job is done
	printf("\n");

	printf("Notifying workers to terminate...\n");
	
	for(i=0; i<nOfThreads; i++)
	{
		pthread_mutex_unlock(&mutex);
	}
	
	for(threadIndex=0; threadIndex<=nOfThreads-1; threadIndex++)
	{
		pthread_join(thread[threadIndex], NULL);
	} 

	printf("Terminating...\n");
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);

	return 0;
}

int primetest(int v) //If not prime return 0, if prime return 1
{
	int i;
	
	for(i=2; i<v; i++)
	{
		if(v==1 || v==2){
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
	int isprime,numberInThisThread;
	int i=(int)(long long int) ptr;

	while(1)
	{	
 		//Wait for a new number that needs to be checked
		pthread_mutex_lock(&mutex);

		if(!JobIsDone)
		{
			if(number == currentNumber) //Trying to check a number already checked
			{
				pthread_mutex_unlock(&mutex);
				continue;
			}
		}
		else //No numbers left
		{
			printf("Thread %d can now terminate\n", i+1);
			pthread_mutex_unlock(&mutex);
			break;
		}

		currentNumber = number;
		numberInThisThread = currentNumber;

 		printf("Thread %d, checking number %d\n", i+1, numberInThisThread);

		isprime = primetest(numberInThisThread); //The prime check is executed outside the CS
		if (isprime == 1)
		{
			printf("[Number %d is prime!]\n",numberInThisThread);
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}