// ./coroutinecheck a.txt

#include <stdio.h>
#include <string.h>
#include <ucontext.h>

#include "mycoroutines.c"

void readFromFile();
void writeToFile();
void fileCompare();

static ucontext_t *c1, *c2, *c3; 

char readFileName[21];
int bufferSize = 20;
char buffer[20];

int main(int argc, char *argv[])
{
	c1 = malloc(sizeof(ucontext_t));
	c2 = malloc(sizeof(ucontext_t));
	c3 = malloc(sizeof(ucontext_t));

	strcpy(readFileName, argv[1]);
	
	mycoroutines_init(c1);
	mycoroutines_create(c2, readFromFile, readFileName);
	mycoroutines_create(c3, writeToFile, NULL);

	mycoroutines_switchto(c2);

	printf("Job complete!\n");

	mycoroutines_destroy(c1);
	mycoroutines_destroy(c2);
	mycoroutines_destroy(c3);

	//comparing the two files
	fileCompare();

	return 0;
}

void readFromFile()
{
	int i;
	static char c;

	FILE *file;
	file = fopen(readFileName, "r");
	
	if(file)
	{	
		i = 0;
		
		while((c = getc(file)) != EOF)
		{
			buffer[i] = c;	//getting one character at a time from the file and writing it into the buffer
			printf("writing %c to buffer[%d]\n", c, i);
			i++;

			
			mycoroutines_init(c2);	//saving the state of the coroutine before checking
			if(i == 20)
			{
				printf("buffer is full!\n");
				printf("%s\n", buffer);
				i = 0;	//resetting the buffer counter	
				mycoroutines_switchto(c3);	//the buffer is full so we change to the writing coroutine
			}	
		}
		fclose(file);
		
		mycoroutines_init(c2); //saving the state of the coroutine before checking
		if(i > 0) //EOF but characters have been written into the buffer
		{
			printf("end of file but buffer isnt full!\n");
			bufferSize = i; //changing this so that writeToFile only reads up to the point of the buffer that characters exist 
			i=0;
			mycoroutines_switchto(c3);
		}
		mycoroutines_switchto(c1);
	}	
}

void writeToFile()
{
	int i;
	FILE *file;
	file = fopen("output.txt", "a");

	for(i = 0; i<bufferSize; i++)
	{
		fputc(buffer[i], file);
		printf("reading %c from buffer[%d]\n", buffer[i], i);
	}
	
	fclose(file);
	mycoroutines_switchto(c2);
}

void fileCompare()
{
	FILE *fileA = fopen(readFileName, "r");
	FILE *fileB = fopen("output.txt", "r");
	char c1, c2;
	int errors = 0;

	while(c1 != EOF && c2 != EOF)
	{
		c1 = getc(fileA);
		c2 = getc(fileB);
		if(c1 != c2)
		{
			errors++;
		}
	}

	if(errors == 0)
	{
		printf("No differences between the 2 files\n");
	}
	else
	{
		printf("%d was the number of differences between the files\n", errors);
	}

	fclose(fileA);
	fclose(fileB);
}