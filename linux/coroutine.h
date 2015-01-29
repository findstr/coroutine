#ifndef _COROUTINE_H
#define _COTOUTINE_H


struct coroutine;

#define COROUTINE_SUSPEND       1
#define COROUTINE_RUNNING       2
#define COROUTINE_DEAD          3
#define COROUTINE_NORMAL        4

typedef void (coroutine_func_t)(struct coroutine *c, void *arg);


struct coroutine *coroutine_create();
void coroutine_free(struct coroutine *C);

int coroutine_push(struct coroutine *C, coroutine_func_t *func, void *arg);

void coroutine_yield(struct coroutine *C);
void coroutine_resume(struct coroutine *C, int id);
int coroutine_status(struct coroutine *C, int id);
int coroutine_running(struct coroutine *C);

#endif


