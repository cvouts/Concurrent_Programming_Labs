// ./train 3 16

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *train_function();
void *passenger_function();

int maxNumberOfPassengers, currentNumberOfPassengers, allPassengers, allPassengersCount, k;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ableToBoard = PTHREAD_COND_INITIALIZER;
pthread_cond_t waitForBoarding = PTHREAD_COND_INITIALIZER;
pthread_cond_t waitForUnboarding = PTHREAD_COND_INITIALIZER;
pthread_cond_t waitForTripToEnd = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
	int trainThreadCheck, passengerThreadCheck, i;
	maxNumberOfPassengers = atoi(argv[1]); //Getting the number of passengers as an argument
	allPassengers = atoi(argv[2]);

	allPassengersCount = allPassengers;

	pthread_t trainThread, passengerThreads[allPassengersCount];

	//Creating the threads
	for(i=0; i<allPassengersCount; i++)
	{
		passengerThreadCheck = pthread_create(&passengerThreads[i], NULL, passenger_function, NULL);
		if(passengerThreadCheck)
		{
			fprintf(stderr,"Error - pthread_create() in passengers return code: %d\n", passengerThreadCheck);
			exit(EXIT_FAILURE);
		}
	}
	trainThreadCheck = pthread_create(&trainThread, NULL, train_function, NULL);
	if(trainThreadCheck)
	{
		fprintf(stderr,"Error - pthread_create() return code: %d\n", trainThreadCheck);
		exit(EXIT_FAILURE);
	}	
	
	pthread_join(trainThread, NULL);
	for(i=0; i<allPassengersCount; i++)
	{
		pthread_join(passengerThreads[i], NULL);
	}

	return 0;
}

void *train_function()
{
	int numberOfTrips, i, j;

	numberOfTrips = allPassengersCount / maxNumberOfPassengers;
	if(allPassengersCount % maxNumberOfPassengers > 0)
	{
		numberOfTrips++;
	} 

	printf("There's going to be %d trips!\n", numberOfTrips);

	for(i=0; i<numberOfTrips; i++)
	{	
		pthread_mutex_lock(&mutex);
		printf(">Potential passengers: %d\n", allPassengers);
		for(j=0; j<maxNumberOfPassengers; j++)
		{
			pthread_cond_signal(&ableToBoard); //Signal that they can board
		}
		
		pthread_cond_wait(&waitForBoarding, &mutex); //Wait until they board
		pthread_mutex_unlock(&mutex);

		printf("Starting new trip...\n");
		sleep(1);
		printf("...Trip is over!\n");

		pthread_mutex_lock(&mutex);
		for(j=0; j<maxNumberOfPassengers; j++)
		{
			pthread_cond_signal(&waitForTripToEnd); //Signal that the trip is over
		}
		
		pthread_cond_wait(&waitForUnboarding, &mutex); //Wait until they unboard
		pthread_mutex_unlock(&mutex);
	}
	return 0;	
}

void *passenger_function()
{
	pthread_mutex_lock(&mutex); 
	pthread_cond_wait(&ableToBoard, &mutex); //Wait until you can board

	currentNumberOfPassengers++;
	printf("Current number of passengers: %d\n", currentNumberOfPassengers);
	
	if(currentNumberOfPassengers == maxNumberOfPassengers || currentNumberOfPassengers == allPassengers)
	{
		printf("Everyone is on board!\n");
		pthread_cond_signal(&waitForBoarding); //Signal that everyone is on board	
	}
	pthread_mutex_unlock(&mutex);
	
	pthread_mutex_lock(&mutex); 
	pthread_cond_wait(&waitForTripToEnd, &mutex); //Wait for the trip to end
	
	currentNumberOfPassengers--;
	allPassengers--;
	printf("Current number of passengers: %d\n", currentNumberOfPassengers);	

	if(currentNumberOfPassengers==0)
	{
		printf("Everyone is off board!\n");
		pthread_cond_signal(&waitForUnboarding); //Signal that everyone is off board
	}
	pthread_mutex_unlock(&mutex);
	
	return 0;
}