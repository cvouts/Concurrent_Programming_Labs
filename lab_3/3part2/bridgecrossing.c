// ./bridgecrossing 4 5 1 3 - blueCars redCars timeToCross numberOfCarsAllowedOnBridge

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *bluecar_function();
void *redcar_function();

int blueCars, redCars, timeToCross, blueCarsOnTheBridge, redCarsOnTheBridge, numberOfCarsAllowedOnBridge, i;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t addingBlue = PTHREAD_COND_INITIALIZER;
pthread_cond_t addingRed = PTHREAD_COND_INITIALIZER;


int main(int argc, char *argv[])
{
	int blueThreadCheck, redThreadCheck, numberOfBlueCarThreads, numberOfRedCarThreads;

	blueCars = atoi(argv[1]);
	redCars = atoi(argv[2]);
	timeToCross = atoi(argv[3]);
	numberOfCarsAllowedOnBridge = atoi(argv[4]);
	
	numberOfBlueCarThreads = blueCars;
	numberOfRedCarThreads = redCars;

	pthread_t blueCarThreads[numberOfBlueCarThreads], redCarThreads[numberOfRedCarThreads];


	//Creating blue and red car threads respectively
	for(i=0; i<numberOfBlueCarThreads; i++)
	{
		blueThreadCheck = pthread_create(&blueCarThreads[i], NULL, bluecar_function, NULL);
		if(blueThreadCheck)
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n", blueThreadCheck);
			exit(EXIT_FAILURE);
		}
	}
	for(i=0; i<numberOfRedCarThreads; i++)
	{
		redThreadCheck = pthread_create(&redCarThreads[i], NULL, redcar_function, NULL);
		if(redThreadCheck)
		{
			fprintf(stderr,"Error - pthread_create() return code: %d\n", redThreadCheck);
			exit(EXIT_FAILURE);
		}
	}

	//Waiting for all the cars to cross before terminating
	for(i=0;i<numberOfBlueCarThreads;i++){
		pthread_join(blueCarThreads[i],NULL);
	}

	for(i=0;i<numberOfRedCarThreads;i++){
		pthread_join(redCarThreads[i],NULL);
	}

	return 0;
}

void *bluecar_function()
{
	while(1)
	{	
		pthread_mutex_lock(&mutex);

		if((redCarsOnTheBridge > 0) || (blueCarsOnTheBridge >= numberOfCarsAllowedOnBridge))
		{
			pthread_cond_wait(&addingBlue, &mutex);
		}
	
		blueCarsOnTheBridge++;

		if(blueCarsOnTheBridge == 1)
		{
			printf("BLUE CARS ON THE BRIDGE\n");
		}

		printf("Added a blue car, %d is the number of cars on the bridge\n", blueCarsOnTheBridge);
		pthread_mutex_unlock(&mutex);

		sleep(timeToCross);

		pthread_mutex_lock(&mutex);
		blueCarsOnTheBridge--;
		blueCars--;
		printf("Removed a blue car, %d is the number of cars left\n", blueCarsOnTheBridge);
		
		if(redCars > 0 && blueCarsOnTheBridge == 0) 
		{
			for(i=0; i<numberOfCarsAllowedOnBridge; i++)
			{
				pthread_cond_signal(&addingRed);
			}
		}
		else if(redCars == 0 && blueCarsOnTheBridge == 0)
		{
			for(i=0; i<numberOfCarsAllowedOnBridge; i++)
			{
				pthread_cond_signal(&addingBlue);
			}
		}
		pthread_mutex_unlock(&mutex);	
		
		break;
	}
	return 0;
}

void *redcar_function()
{
	while(1)
	{	
		pthread_mutex_lock(&mutex);

		if((blueCarsOnTheBridge>0)||(redCarsOnTheBridge >= numberOfCarsAllowedOnBridge))
		{
			pthread_cond_wait(&addingRed, &mutex);
		}

		redCarsOnTheBridge++;

		if(redCarsOnTheBridge == 1)
		{
			printf("RED CARS ON THE BRIDGE\n");
		}

		printf("Added a red car, %d is the number of cars on the bridge\n", redCarsOnTheBridge);
		pthread_mutex_unlock(&mutex);

		sleep(timeToCross);

		pthread_mutex_lock(&mutex);
		redCarsOnTheBridge--;
		redCars--;
		printf("Removed a red car, %d is the number of cars left\n", redCarsOnTheBridge);
			
		if(blueCars > 0 && redCarsOnTheBridge == 0) 
		{
			for(i=0; i<numberOfCarsAllowedOnBridge; i++)
			{
				pthread_cond_signal(&addingBlue);
			}
		}
		else if(blueCars == 0 && redCarsOnTheBridge == 0)
		{
			for(i=0; i<numberOfCarsAllowedOnBridge; i++)
			{
				pthread_cond_signal(&addingRed);
			}
		}

		pthread_mutex_unlock(&mutex);
		break;
	}
	return 0;
}