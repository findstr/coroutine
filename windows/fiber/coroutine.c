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
#include <Windows.h>

#include "coroutine.h"

#define my_malloc(a)                    malloc(a)
#define my_free(a)                      free(a)
#define my_realloc(ptr, size)           realloc(ptr, size)

#define STACK_SIZE                      (1024 * 1024)

struct task {
        int                     status;
        struct coroutine        *C;
        void                    *arg;
	void			(*func)(void *arg);
        LPVOID                  fiber;
};

struct coroutine {
        int                     running;
        LPVOID                  fiber;
        int                     task_cnt;
        int                     task_cap;       //TODO:if many task, optimise it
        struct task             **task_list;    //for more cache line hit rate
};

static void
_del_task(struct task *T)
{
	DeleteFiber(T->fiber);
	my_free(T);
}

static void WINAPI
_mainfunc(struct task *arg)
{
	assert(arg);
	
	arg->func(arg->arg);
	
	SwitchToFiber(arg->fiber);
}

static struct task *
_new_task(struct coroutine *C, coroutine_func_t *func, void *arg)
{
        struct task *t;
        
        t = (struct task *)my_malloc(sizeof(*t));
 
	t->status = COROUTINE_NORMAL;
        t->C = C;
        t->arg = arg;
        t->func = func;
        t->fiber = CreateFiber(0, _mainfunc, t); 

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
	C->fiber = ConvertThreadToFiber(C);

        return C;
}

void coroutine_free(struct coroutine *C)
{
        int i;

        assert(C);

        for (i = 0; i < C->task_cnt; i++)
                my_free(C->task_list[i]);

	DeleteFiber(C->fiber);
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

void coroutine_yield()
{
        struct coroutine *C;

        C = ((struct task*)GetFiberData())->C;

        assert(C);
        assert(C->running >= 0);

        C->task_list[C->running]->status = COROUTINE_SUSPEND;
        SwitchToFiber(C->fiber);

        return ;
}

void coroutine_resume(struct coroutine *C, int id)
{
        assert(C);

        C->running = id;
        assert(id < C->task_cnt);

        C->task_list[id]->status = COROUTINE_RUNNING;
        
        SwitchToFiber(C->task_list[id]->fiber);

        if (C->task_list[id]->status == COROUTINE_RUNNING) {
                _del_task(C->task_list[id]);
                C->task_list[id] = NULL;
        }

        return ;
}

int coroutine_status(struct coroutine *C, int id)
{
        assert(C);
        assert(id < C->task_cnt);
        assert(C->task_list[id]);
        if (C->task_list[id] == NULL)
                return COROUTINE_DEAD;

        return C->task_list[id]->status;
}

int coroutine_running(struct coroutine *C)
{
        assert(C);
        return C->running;
}

