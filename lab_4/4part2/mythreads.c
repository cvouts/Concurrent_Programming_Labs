#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <unistd.h>

#include "mycoroutines.c"

typedef struct Thread
{
	int threadNumber;
	int isAvailable;
	int isTerminated;
	ucontext_t* context;
} thr_t;

typedef struct Semaphore
{
	int value;
} sem_t;


#define MAXNUMBEROFTHREADS 100
#define THREADCHANGEINTERVALINMSECONDS 10000

struct Thread* threadArray[MAXNUMBEROFTHREADS];
int threadCounter, threadsTerminated, curThread = 0, yieldCounter = 1;
ucontext_t *maincontext, *handlerContext, *curContext;
void *signalStack;
sigset_t set, semset;

void mythreads_scheduler();
void mythreads_thread_handler();
void mythreads_setup_signals();
void mythreads_setup_timer();
void mythreads_yield_signal_initialization();
void mythreads_yield_signal_handler();
void mythreads_handle_thread_termination();

//setting the thread of the main function (at threadArray[0])
//allocating memory for stuff, setting the atexit() fuction calls
//calling functions
int mythreads_init()
{
	threadCounter = 0;
	threadArray[0] = malloc(sizeof(thr_t));
	threadArray[0]->threadNumber = threadCounter;
	threadArray[0]->isAvailable = 1;
	threadArray[0]->context = malloc(sizeof(ucontext_t));

	threadCounter = 1;

	handlerContext = malloc(sizeof(ucontext_t));
	curContext = malloc(sizeof(ucontext_t));

	signalStack = malloc(SIGSTKSZ);
    if (signalStack == NULL)
    {
        perror("malloc");
        exit(1);
    }

 	for(int j=0; j<MAXNUMBEROFTHREADS; j++)
	{
		atexit(mythreads_handle_thread_termination);
	}

	mythreads_yield_signal_initialization();
    mythreads_setup_signals();
    mythreads_setup_timer();

    getcontext(threadArray[0]->context);
	return 0;
}

//creating the threads and setting their context
int mythreads_create(thr_t *thread, void (body) (void), void *arg)
{
	threadArray[threadCounter] = malloc(sizeof(thr_t));
	threadArray[threadCounter] = thread;

	threadArray[threadCounter]->threadNumber = threadCounter;
	threadArray[threadCounter]->isAvailable = 1;
	threadArray[threadCounter]->context = malloc(sizeof(ucontext_t));

	mycoroutines_create(threadArray[threadCounter]->context, body, NULL);

	printf("Created thread number %d\n", threadArray[threadCounter]->threadNumber);

	threadCounter++;
	return 0;
}

//initializing the signal SIGALRM that is used for swapping the threads
void mythreads_setup_signals()
{
    struct sigaction act;

    act.sa_sigaction = mythreads_thread_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);

    if(sigaction(SIGALRM, &act, NULL) != 0) 
    {
        perror("Signal handler");
    }
}

//timer for the thread swapping
void mythreads_setup_timer()
{
	struct itimerval it;  

	//it.it_interval.tv_sec = THREADCHANGEINTERVAL;
    it.it_interval.tv_usec = THREADCHANGEINTERVALINMSECONDS;
    it.it_value = it.it_interval;
    if (setitimer(ITIMER_REAL, &it, NULL) ) perror("setitimer");
}

//setting and swapping to the scheduler
void mythreads_thread_handler(int j, siginfo_t *si, void *old_context)
{
	getcontext(handlerContext);
	
    handlerContext->uc_stack.ss_sp = signalStack;
    handlerContext->uc_stack.ss_size = SIGSTKSZ;
    handlerContext->uc_stack.ss_flags = 0;
    sigemptyset(&handlerContext->uc_sigmask);
 
    makecontext(handlerContext, mythreads_scheduler, 1);
   
    //save running thread, swap to scheduler
    swapcontext(curContext, handlerContext);
}

void mythreads_scheduler()
{

	if(threadArray[(curThread + 1) % threadCounter]->isAvailable == 1)
    {
    	curThread = (curThread + 1) % threadCounter; // round robin
	}
	else //skip non available threads
	{
		curThread = (curThread + 2) % threadCounter;
		while(curThread <= threadCounter)
		{
			if(threadArray[curThread]->isAvailable == 1)
			{
				break;
			}
			else curThread = (curThread+1) % threadCounter;
		}
	}
   
	//printf("switching to context of %d\n", curThread);
	//switching to context of curThread
    curContext = threadArray[curThread]->context;
    setcontext(curContext);
}

//waits in the while loop for the given thread to terminate
void mythreads_join(thr_t *thr)
{
	while(thr->isTerminated != 1) {}
	if(thr->isTerminated == 1)
	{
		printf("Thread %d has terminated!\n", thr->threadNumber);
	}
}

//for the interval specified, the thread that called this function 
//is skipped in the round robin of the scheduler
void mythreads_yield()
{
 	threadArray[curThread]->isAvailable = 0;
 	struct itimerval it;  

	//it.it_interval.tv_sec = THREADCHANGEINTERVAL;
    it.it_interval.tv_usec = 5*THREADCHANGEINTERVALINMSECONDS;
    it.it_value = it.it_interval;
    if (setitimer(ITIMER_VIRTUAL, &it, NULL) ) perror("setitimer");
}

void mythreads_destroy(thr_t *thr)
{
	threadCounter--;
	free(thr->context);
	free(thr);
	printf("Thread destroyed!\n");
}

int mythreads_sem_init(sem_t *s, int val)
{
	s->value = val;

	return 0;
}

int mythreads_sem_down(sem_t *s)
{
	if(s->value > -1)
	{
		s->value--;
	}

	while(s->value == -1){} //this is where a semaphore blocks
	return 0;
}

int mythreads_sem_up(sem_t *s)
{
	s->value++;
	return 0;
}

int mythreads_sem_destroy(sem_t *s)
{
	printf("Semaphore destroyed!\n");
	free(s);

	return 0;
}

//initializing the signal SIGVTALRM that is used for the yield function
void mythreads_yield_signal_initialization()
{
	struct sigaction act2;
	act2.sa_sigaction = mythreads_yield_signal_handler;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&semset);
    sigaddset(&semset, SIGVTALRM);

    if(sigaction(SIGVTALRM, &act2, NULL) != 0) 
    {
        perror("Signal handler");
    }
}

//handling the above SIGVTALRM signal
void mythreads_yield_signal_handler(int signal)
{
	while(yieldCounter<threadCounter)
	{
		if(threadArray[yieldCounter]->isAvailable == 0)
		{
			threadArray[yieldCounter]->isAvailable = 1;
			yieldCounter++;


			//when the max number of threads is reached, it resets to the first
			//because that is the next one that is going to yield
			if(yieldCounter >= threadCounter)
			{
				yieldCounter = 1;
			}
			break;
		}
	}
}

//the function called by the atexit() for when the threads terminate
void mythreads_handle_thread_termination()
{
	threadArray[curThread]->isAvailable = 0;
	threadArray[curThread]->isTerminated = 1;
	threadsTerminated++;
	if(threadsTerminated < threadCounter)
	{
		threadArray[curThread-1]->isAvailable = 1;
		swapcontext(curContext, handlerContext);
	}
}