#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#ifndef MYTHREADS_H
#define MYTHREADS_H

typedef struct Thread
{
	int threadNumber;
	int isAvailable;
	ucontext_t* context;
} thr_t;

typedef struct Semaphore
{
	int ID;
	int value;

} sem_t;

extern thr_t* threadArray[100];
extern int curThread;

int mythreads_init();
int mythreads_create(thr_t *thread, void (body) (void), void *arg);
void mythreads_setup_signals();
void mythreads_setup_timer();
void mythreads_thread_handler(int j, siginfo_t *si, void *old_context);
void mythreads_scheduler();
void mythreads_join();
void mythreads_yield();
void mythreads_destroy(thr_t *thr);
int mythreads_sem_init(sem_t *s, int val);
int mythreads_sem_down(sem_t *s);
int mythreads_sem_up(sem_t *s);
int mythreads_sem_destroy(sem_t *s);
void mythreads_yield_signal_initialization();
void mythreads_yield_signal_handler(int signal);
void mythreads_termination_signal();
void mythreads_handle_thread_termination();

#endif

