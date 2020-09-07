#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>

ucontext_t* thisContext; 

int mycoroutines_init(ucontext_t *co)
{
	getcontext(co);
	return 0;
}

int mycoroutines_create(ucontext_t *co, void (body) (void), void *arg)
{
	getcontext(co);
	co->uc_stack.ss_size = SIGSTKSZ;
	co->uc_stack.ss_sp = malloc(1024*1024*8);
	co->uc_stack.ss_flags = 0;
    if (sigemptyset(&co->uc_sigmask) < 0)
    {
      perror("sigemptyset");
      exit(1);
    }
	makecontext(co, body, 1, arg);

	return 0;
}

int mycoroutines_switchto(ucontext_t *co)
{
	thisContext = malloc(sizeof(ucontext_t));
	//getcontext(thisContext);

	swapcontext(thisContext, co);
	return 0;
}

int mycoroutines_destroy(ucontext_t *co)
{
	free(co);
	return 0;
}

