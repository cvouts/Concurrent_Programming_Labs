// ./bridgecrossing 4 5 1 3 - blueCars redCars timeToCross numberOfCarsAllowedOnBridge

#define SEM1_KEY "../2part1/ftok.txt"
#define SEM2_KEY "../2part1/ftok2.txt"
#define SEM3_KEY "../2part1/ftok3.txt"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "../2part1/mysem.c"

void *bluecar_function();
void *redcar_function();

key_t key1, key2, key3;
int addedToTheBridgeSemID, bridgeControlSemID, removedFromTheBridgeSemID;
int blueCars, redCars, timeToCross, blueCarsOnTheBridge, redCarsOnTheBridge, numberOfCarsAllowedOnBridge;

int main(int argc, char *argv[])
{
	int blueThreadCheck, redThreadCheck, i;

	blueCars = atoi(argv[1]);
	redCars = atoi(argv[2]);
	timeToCross = atoi(argv[3]);
	numberOfCarsAllowedOnBridge = atoi(argv[4]);
	
	pthread_t blueCarThreads[blueCars], redCarThreads[redCars];

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

	addedToTheBridgeSemID = mysem_create(key1);	//This ensures that only one car is added to the bridge at a time
	bridgeControlSemID = mysem_create(key2); //This ensures that only one type of car is on the bridge at a time
	removedFromTheBridgeSemID = mysem_create(key3); //This ensures that only one car is removed from the bridge at a time

	//Creating blue and red car threads respectively
	for(i=0; i<blueCars; i++)
	{
		blueThreadCheck = pthread_create(&blueCarThreads[i], NULL, bluecar_function, NULL);
		if(blueThreadCheck)
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n", blueThreadCheck);
			exit(EXIT_FAILURE);
		}
	}
	for(i=0; i<redCars; i++)
	{
		redThreadCheck = pthread_create(&redCarThreads[i], NULL, redcar_function, NULL);
		if(redThreadCheck)
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n", redThreadCheck);
			exit(EXIT_FAILURE);
		}
	}

	//Waiting for all the cars to cross before terminating
	for(i=0;i<blueCars;i++){
		pthread_join(blueCarThreads[i],NULL);
	}

	for(i=0;i<redCars;i++){
		pthread_join(redCarThreads[i],NULL);
	}

	mysem_destroy(addedToTheBridgeSemID);
	mysem_destroy(bridgeControlSemID);
	mysem_destroy(removedFromTheBridgeSemID);

	return 0;
}

void *bluecar_function()
{
	while(1)
	{	
		mysem_down(addedToTheBridgeSemID); //Only one car is added at a time
		if(blueCarsOnTheBridge == 0)
		{
			mysem_down(bridgeControlSemID); //The bridge is empty and the first car on it is a blue one
			printf("BLUE CARS ON THE BRIDGE\n");
		}

		if(blueCarsOnTheBridge >= numberOfCarsAllowedOnBridge)
		{	
			mysem_up(addedToTheBridgeSemID); //Max number of cars is reached
			continue;
		}	
		
		blueCarsOnTheBridge++;
		printf("Added a blue car, %d is the number of cars on the bridge\n", blueCarsOnTheBridge);
		mysem_up(addedToTheBridgeSemID); //A new car can try to go on the bridge
		sleep(timeToCross);

		mysem_down(removedFromTheBridgeSemID); //Only one car is removed at a time
		blueCarsOnTheBridge--;
		printf("Removed a blue car, %d is the number of cars left\n", blueCarsOnTheBridge);
		mysem_up(removedFromTheBridgeSemID);
		if(blueCarsOnTheBridge == 0) //If the bridge is empty, cede control to whichever kind of car comes next
		{
			printf(">>Ceding bridge control\n");
			mysem_up(bridgeControlSemID);
		}
		break;
	}
	return 0;
}

void *redcar_function()
{
	while(1)
	{	
		mysem_down(addedToTheBridgeSemID); //Down so that no other car is added at the same time
		if(redCarsOnTheBridge == 0)
		{
			mysem_down(bridgeControlSemID); //The bridge is empty and the first car on it is a red one
			printf("RED CARS ON THE BRIDGE\n");
		}
		
		if(redCarsOnTheBridge >= numberOfCarsAllowedOnBridge)
		{	
			mysem_up(addedToTheBridgeSemID); //Max number of cars is reached
			continue;
		}	
			
		redCarsOnTheBridge++;
		printf("Added a red car, %d is the number of cars on the bridge\n", redCarsOnTheBridge);
		mysem_up(addedToTheBridgeSemID); //A new car can try to go on the bridge
		sleep(timeToCross);

		mysem_down(removedFromTheBridgeSemID);
		redCarsOnTheBridge--;
		printf("Removed a red car, %d is the number of cars left\n", redCarsOnTheBridge);
		mysem_up(removedFromTheBridgeSemID); 

		if(redCarsOnTheBridge == 0) //If the bridge is empty, cede control to whichever kind of car comes next
		{
			printf(">>Ceding bridge control\n");
			mysem_up(bridgeControlSemID);
		}
		break;
	}
	return 0;
}