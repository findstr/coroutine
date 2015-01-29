/**
=========================================================================
 Author: findstr
 Email: findstr@sina.com
 File Name: coroutine.c
 Description: (C)  2015-01  findstr
   
 Edit History: 
   2015-01-28    File created.
=========================================================================
**/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ucontext.h>

#include "coroutine.h"

#define my_malloc(a)                    malloc(a)
#define my_free(a)                      free(a)
#define my_realloc(ptr, size)           realloc(ptr, size)

#define STACK_SIZE                      (1024 * 1024)

struct task {
        int                     status;
        struct ucontext         ctx;
        unsigned char           stack[STACK_SIZE];
};

struct coroutine {
        int                     running;
        struct ucontext         ctx;
        unsigned char           stack[STACK_SIZE];
        int                     task_cnt;
        int                     task_cap;       //TODO:if many task, optimise it
        struct task             **task_list;    //for more cache line hit rate
};

static struct task *
_new_task(struct coroutine *C, coroutine_func_t *func, void *arg)
{
        struct task *t;
        
        t = (struct task *)my_malloc(sizeof(*t));
 
        getcontext(&t->ctx);

        t->status = COROUTINE_NORMAL;
        t->ctx.uc_link = &C->ctx;
        t->ctx.uc_stack.ss_sp = t->stack;
        t->ctx.uc_stack.ss_size = sizeof(t->stack);
        
        makecontext(&t->ctx, (void(*)())func, 2, C, arg);
        
        return t;
}

static int 
_add_task(struct coroutine *C, struct task *t)
{
        assert(t);

        if (C->task_cnt >= C->task_cap) {
                C->task_cap = C->task_cap * 2 + 1;
                C->task_list = my_realloc(C->task_list, C->task_cap * sizeof(*t));
        }

        C->task_list[C->task_cnt] = t;

        return C->task_cnt++;
}

struct coroutine *coroutine_create()
{
        struct coroutine *C;

        C = (struct coroutine *)my_malloc(sizeof(*C));
        memset(C, 0, sizeof(*C));
        C->running = -1;

        return C;
}

void coroutine_free(struct coroutine *C)
{
        int i;

        assert(C);

        for (i = 0; i < C->task_cnt; i++)
                my_free(C->task_list[i]);

        my_free(C->task_list);
        my_free(C);
}

int coroutine_push(struct coroutine *C, coroutine_func_t *func, void *arg)
{
        int id;
        struct task *t;

        t = _new_task(C, func, arg);
        id = _add_task(C, t);

        return id;
}

void coroutine_yield(struct coroutine *C)
{
        int err;
        
        assert(C->running >= 0);

        C->task_list[C->running]->status = COROUTINE_SUSPEND;
        swapcontext(&C->task_list[C->running]->ctx, &C->ctx);

        return ;
}

void coroutine_resume(struct coroutine *C, int id)
{
        assert(C);

        C->running = id;
        assert(id < C->task_cnt);

        C->task_list[id]->status = COROUTINE_RUNNING;
        
        swapcontext(&C->ctx, &C->task_list[id]->ctx);

        if (C->task_list[id]->status == COROUTINE_RUNNING)
                C->task_list[id]->status = COROUTINE_DEAD;

        return ;
}

int coroutine_status(struct coroutine *C, int id)
{
        assert(C);
        assert(id < C->task_cnt);
        assert(C->task_list[id]);
        return C->task_list[id]->status;
}

int coroutine_running(struct coroutine *C)
{
        assert(C);
        return C->running;
}

