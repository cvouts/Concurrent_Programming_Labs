//gcc -Wall mysem.c -o mysem -c

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>

union semun  
{
	int val;
	struct semid_ds *buf;
	ushort array [1];
} sem_attr;

int mysem_create(key_t semKey)
{
	int binarySemID;
	if ((binarySemID = semget (semKey, 1, 0660 | IPC_CREAT)) == -1) 
	{
		perror ("semget");
		return -1;
	}

	//Initializing
	sem_attr.val = 1;  
	if (semctl (binarySemID, 0, SETVAL, sem_attr) == -1) 
	{
		perror ("semctl initial SETVAL"); 
		return -1;
	}
	return binarySemID;
}

void mysem_down(int binarySemID)
{	
	struct sembuf asem [1];
	asem [0].sem_num = 0;
	asem [0].sem_flg = 0;

	asem [0].sem_op = -1;

	semop (binarySemID, asem, 1);
}

void mysem_up(int binarySemID)
{
	struct sembuf asem [1];
	asem [0].sem_num = 0;
	asem [0].sem_flg = 0;

	asem [0].sem_op = 1;

	semop (binarySemID, asem, 1);
	if(semctl(binarySemID, 0, GETVAL) == 2)
	{
		printf("up called for a semaphore that already had a value of 1\n");
		semop (binarySemID, asem, 1);
	}
}

void mysem_destroy(int binarySemID)
{
	if (semctl (binarySemID, 0, IPC_RMID) == -1)
	{
		perror ("semctl IPC_RMID"); 
		exit (1);
	}
}
