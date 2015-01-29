// fiber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>

#include "coroutine.h"

#define COROUTINE_CNT   100


VOID func1(LPVOID lpFiberParameter)
{
        int i;
        for (i = 0; i < 10; i++) {
                printf("i:%d\n", i);
                coroutine_yield();
        }
}

int _tmain(int argc, _TCHAR* argv[])
{
        int i;
        int F[COROUTINE_CNT];
        struct coroutine *C;

        C = coroutine_create();

        for (i = 0; i < COROUTINE_CNT; i++) {
                F[i] = coroutine_push(C, func1, NULL);
        }

        printf("mother fiber ok.\n");

        coroutine_resume(C, F[0]);
        printf("create fiber ok.\n");
        coroutine_resume(C, F[0]);
        printf("create fiber ok.\n");
        coroutine_resume(C, F[0]);
        printf("create fiber ok.\n");
        coroutine_resume(C, F[0]);
        printf("create fiber ok.\n");
        coroutine_resume(C, F[0]);
        printf("create fiber ok.\n");
        coroutine_resume(C, F[0]);
        printf("create fiber ok.\n");

        fflush(stdout);

        for (;;)
                Sleep(5000);

	return 0;
}

