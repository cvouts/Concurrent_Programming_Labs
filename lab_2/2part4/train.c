// ./train 3

#define SEM1_KEY "../2part1/ftok.txt"
#define SEM2_KEY "../2part1/ftok2.txt"
#define SEM3_KEY "../2part1/ftok3.txt"
#define SEM4_KEY "../2part1/ftok4.txt"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "../2part1/mysem.c"

void *train_function();
void *passenger_function();

key_t key1, key2, key3, key4;
int trainWaitSemID, boardingSemID, trainTravelSemID, unboardingSemID;
int maxNumberOfPassengers, currentNumberOfPassengers;

int main(int argc, char *argv[])
{
	int trainThreadCheck, passengerThreadCheck, i;
	maxNumberOfPassengers = atoi(argv[1]); //Getting the number of passengers as an argument
	pthread_t trainThread, passangerThreads[maxNumberOfPassengers];

	//Creating the keys for the semaphores
	if ((key1 = ftok (SEM1_KEY, 'a')) == -1) 
	{
		perror ("ftok 1"); exit (1);
	}
	if ((key2 = ftok (SEM2_KEY, 'a')) == -1) 
	{
		perror ("ftok 2"); exit (1);
	}
	if ((key3 = ftok (SEM3_KEY, 'a')) == -1) 
	{
		perror ("ftok 3"); exit (1);
	}
	if ((key4 = ftok (SEM4_KEY, 'a')) == -1) 
	{
		perror ("ftok 4"); exit (1);
	}

	boardingSemID = mysem_create(key1); //Each passenger locks this semiphore so that one passenger boards at a time
	trainWaitSemID = mysem_create(key2); //The train locks the semaphore while waiting for the passengers to board, the last passenger unlocks it	
	trainTravelSemID = mysem_create(key3); //The last passenger locks the semaphore and the train unlocks it when the trip is over
	unboardingSemID = mysem_create(key4); //Each passenger locks this semiphore so that one passenger unboards at a time

	//Creating the threads
	trainThreadCheck = pthread_create(&trainThread, NULL, train_function, NULL);
	if(trainThreadCheck)
	{
		fprintf(stderr,"Error - pthread_create() return code: %d\n", trainThreadCheck);
		exit(EXIT_FAILURE);
	}
	mysem_down(trainTravelSemID); //Initialize the semaphore with a value of 0
	for(i=0; i<maxNumberOfPassengers; i++)
	{
		passengerThreadCheck = pthread_create(&passangerThreads[i], NULL, passenger_function, NULL);
		if(passengerThreadCheck)
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n", passengerThreadCheck);
			exit(EXIT_FAILURE);
		}
	}	
	sleep(6);

	mysem_destroy(trainWaitSemID);
	mysem_destroy(boardingSemID);
	mysem_destroy(trainTravelSemID);
	mysem_destroy(unboardingSemID);

	return 0;
}

void *train_function()
{
	mysem_down(trainWaitSemID); //Initialize the semaphore with a value of 0
	while(1)
	{	
		mysem_down(trainWaitSemID); //Waiting for the passengers to board
		printf("Starting new trip...\n");
		sleep(2);
		printf("...Trip is over!\n");
		mysem_up(trainTravelSemID); //The trip is over, new passengers can board
	}
	return 0;	
}

void *passenger_function()
{
	while(1)
	{
		mysem_down(boardingSemID); //Boarding one at a time
		currentNumberOfPassengers++;
		printf("Before trip: Old number of passengers: %d\tCurrent number of passengers: %d\n", currentNumberOfPassengers-1, currentNumberOfPassengers);
		if(currentNumberOfPassengers >= maxNumberOfPassengers)
		{
			printf("Everyone is on board!\n");
			mysem_up(trainWaitSemID); //Train can stop waiting for the passengers to board
		}
		mysem_up(boardingSemID); //Next passenger can board

		mysem_down(trainTravelSemID); //The passenger is waiting for the trip to end to unboard
		mysem_down(unboardingSemID); //Unboarding one at a time
		currentNumberOfPassengers--;
		printf("After trip: Old number of passengers: %d\t\tCurrent number of passengers: %d\n", currentNumberOfPassengers+1, currentNumberOfPassengers);	
		
		if(currentNumberOfPassengers == maxNumberOfPassengers-1)
		{
			mysem_down(boardingSemID); //Locking this so that new passengers have to wait for the previous passengers to unboard before they themselves can board
		}
		if(currentNumberOfPassengers != 0)
		{
			mysem_up(trainTravelSemID);	//The trip is over so the next passenger can unboard
		}
		else
		{
			printf("Everyone is off board!\nNew passengers boarding...\n");
			mysem_up(boardingSemID); //Everyone is off the train, new passengers can board
		}
		mysem_up(unboardingSemID); //Next passenger can unboard
	}
	return 0;
}