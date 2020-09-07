#ifndef MYPIPE_H_
#define MYPIPE_H_

void pipe_init();

void pipe_close(int*);

void *pipe_write();

void *pipe_read();

#endif