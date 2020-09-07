//gcc -Wall mypipe.c -o mypipe -c

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include<fcntl.h> 
#include "mypipe.h"

int fd[2];

void pipe_init(int size)
{	
	int pi;
	pi= pipe(fd);
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);

	//Setting the pipe size to 64
	fcntl(fd[0], F_SETPIPE_SZ, size);
	fcntl(fd[1], F_SETPIPE_SZ, size);

	if(pi<0)
	{
		perror("pipe");
		exit(1);
	}
}

void pipe_close(int fd[2])
{
	close(fd[1]);
	close(fd[0]);
}

void *pipe_write(char *topipe)
{	
	printf("T1: Writing this value into the pipe: %c\n", *topipe);
	write(fd[1], &topipe, sizeof(char));
	
	return 0;
}

void *pipe_read(char *frompipe)
{
	sleep(2);
	
	read(fd[0], &frompipe, sizeof(char));

	printf("T2: value in pipe: %c\n", *frompipe);

	return 0;
}

