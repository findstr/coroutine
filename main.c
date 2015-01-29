/**
=========================================================================
 Author: findstr
 Email: findstr@sina.com
 File Name: main.c
 Description: (C)  2015-01  findstr
   
 Edit History: 
   2015-01-27    File created.
=========================================================================
**/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ucontext.h>

#include "coroutine.h"

struct coroutine;

typedef void (coroutine_func_t)(struct coroutine *c, void *arg);

#define THREAD_CNT      300
#define FOR_CNT         10000

#if THREAD

static int cnt;
static int lock;

static void *
add_func(void *arg)
{
        int a;
        size_t i;
        for (i = 0; i < FOR_CNT; i++) {
                a = 100;
                while (__sync_lock_test_and_set(&lock, 1) == 1)
                        ;

                while (a--)
                        ;

                ((int *)arg)[0]++;

                __sync_lock_release(&lock);
        }
}


int main()
{
        size_t i;
        pthread_t       id[THREAD_CNT];

        lock = 0;

        printf("----------thread------------------\n");

        for (i = 0; i < THREAD_CNT; i++)
                pthread_create(&id[i], NULL, add_func, &cnt);

        for (i = 0; i < THREAD_CNT; i++)
                pthread_join(id[i], NULL);

        printf("All over:%d\n", cnt);

        return 0;
}

#else
static int cnt;

static void
add_func(struct coroutine *C, void *arg)
{
        int a;
        size_t i;
        for (i = 0; i < FOR_CNT; i++) {
                a = 100;
                while (a--)
                        ;
                ((int *)arg)[0]++;
                
                coroutine_yield(C);
        }
}


int main()
{
        size_t i;
        int id[THREAD_CNT];
        struct coroutine *C;

        C = coroutine_create();

        printf("----------------coroutine-------------------\n");

        for (i = 0; i < THREAD_CNT; i++)
                id[i] = coroutine_push(C, add_func, &cnt);

        i = 0;
        for (;;) {
                coroutine_resume(C, id[i]);
                i++;
                i %= THREAD_CNT;

                if (coroutine_status(C, id[THREAD_CNT - 1]) == COROUTINE_DEAD)
                        break;
        }
        printf("All over:%d\n", cnt);

        coroutine_free(C);

        return 0;
}

#endif

