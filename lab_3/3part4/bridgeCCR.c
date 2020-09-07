// ./bridgeCCR 4 5 1 3 - blueCars redCars timeToCross numberOfCarsAllowedOnBridge

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define CCR_DECLARE(label) \
	pthread_mutex_t label ## _mtx; \
	sem_t label ## _q1; \
	sem_t label ## _q2; \
	int label ## _n1; \
	int label ## _n2;

#define CCR_INIT(label) \
	pthread_mutex_init(&label ## _mtx, NULL); \
	sem_init(&label ## _q1, 0, 0); \
	sem_init(&label ## _q2, 0, 0); \
	label ## _n1 = 0; \
	label ## _n2 = 0;

#define CCR_EXEC(label, cond, body) \
	pthread_mutex_lock(&label ## _mtx); \
	while(!cond) \
	{ \
		label ## _n1++; \
		if(label ## _n2 > 0) {label ## _n2--; sem_post(&label ## _q2);} \
        else {pthread_mutex_unlock(&label ## _mtx);}  \
    	sem_wait(&label ## _q1); \
    	label ## _n2++; \
    	if(label ## _n1 > 0) {label ## _n1--; sem_post(&label ## _q1);} \
    	else {sem_post(&label ## _q2);} \
		sem_wait(&label ## _q2); \
	}; \
	body; \
	label ## _n1--; sem_post(&label ## _q1); \
	label ## _n2--; sem_post(&label ## _q2); \
	pthread_mutex_unlock(&label ## _mtx);

	
CCR_DECLARE(ccrAdd);
CCR_DECLARE(ccrRemove);

void *bluecar_function();
void *redcar_function();
void blueBody1();
void blueBody2();
void redBody1();
void redBody2();


int blueCars, redCars, timeToCross, blueCarsOnTheBridge, redCarsOnTheBridge, numberOfCarsAllowedOnBridge, i;
int blueAddCond = 1, redAddCond = 1, blueRemoveCond = 1, redRemoveCond = 1;

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
	
	CCR_INIT(ccrAdd);
	CCR_INIT(ccrRemove);

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


//blueBody1(), blueBody2(), redBody1() and redBody2() are the parts of the functions that are critical sections
//There are also 4 different conditional variables, for adding and removing cars

void *bluecar_function()
{
	CCR_EXEC(ccrAdd, blueAddCond, blueBody1());
	sleep(timeToCross);
	CCR_EXEC(ccrRemove, blueRemoveCond, blueBody2());

	return 0;
}

 void *redcar_function()
 {
 	CCR_EXEC(ccrAdd, redAddCond, redBody1());
	sleep(timeToCross);
 	CCR_EXEC(ccrRemove, redRemoveCond, redBody2());

 	return 0;
 }

void blueBody1()
{
	while(1)
	{
		if((redCarsOnTheBridge > 0) || (blueCarsOnTheBridge >= numberOfCarsAllowedOnBridge))
		{
			blueAddCond = 0;
			continue;
		}

		blueCarsOnTheBridge++;

		if(blueCarsOnTheBridge == 1)
		{
			printf("BLUE CARS ON THE BRIDGE\n");
			redAddCond = 0;
			blueRemoveCond = 1;
		}
		printf("Added a blue car, %d is the number of cars on the bridge\n", blueCarsOnTheBridge);
		break;
	}
}

void blueBody2()
{
	blueCarsOnTheBridge--;
	blueCars--;
	printf("Removed a blue car, %d is the number of cars left\n", blueCarsOnTheBridge);
	sem_post(&ccrAdd_q1);
	sem_post(&ccrAdd_q2);
	if(blueCarsOnTheBridge == 0)
	{
		blueRemoveCond = 0;

		if(redCars > 0)	
		{
			redAddCond = 1;
		}
		else if(redCars == 0)
		{
			blueAddCond = 1;
		}
		return;
	}
}

void redBody1()
{
	while(1)
	{	
		if((blueCarsOnTheBridge > 0) || (redCarsOnTheBridge >= numberOfCarsAllowedOnBridge))
		{
			redAddCond = 0;
			continue;
		}

		redCarsOnTheBridge++;

		if(redCarsOnTheBridge == 1)
		{
			printf("RED CARS ON THE BRIDGE\n");
			blueAddCond = 0;
			redRemoveCond = 1;
		}

		printf("Added a red car, %d is the number of cars on the bridge\n", redCarsOnTheBridge);
		break;
	}
}

void redBody2()
{
	redCarsOnTheBridge--;
	redCars--;
	printf("Removed a red car, %d is the number of cars left\n", redCarsOnTheBridge);
	sem_post(&ccrAdd_q1);
	sem_post(&ccrAdd_q2);
	if(redCarsOnTheBridge == 0)
	{
		redRemoveCond = 0;

		if(blueCars > 0)	
		{
			blueAddCond = 1;
		}
		else if(blueCars == 0)
		{
			redAddCond = 1;
		}
		return;
	}
}