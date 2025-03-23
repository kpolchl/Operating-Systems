#include <stdio.h>
#ifdef DYNAMIC_LOAD
#include <dlfcn.h>
#else
#include "collatz.h"
#endif

#define MAX_ITER 1000

int main()
{
#ifdef DYNAMIC_LOAD
    void *handle = dlopen("./libcollatz.so", RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "Błąd ładowania biblioteki: %s\n", dlerror());
        return 1;
    }
    int (*collatz_conjecture)(int) = dlsym(handle, "collatz_conjecture");
    int (*test_collatz_convergence)(int, int, int *) = dlsym(handle, "test_collatz_convergence");

    if (!collatz_conjecture || !test_collatz_convergence)
    {
        fprintf(stderr, "Błąd pobierania symboli: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
#endif

    int numbers[] = {5, 10, 9, 21, 37, 900, 2137, 90000000};
    int N = sizeof(numbers) / sizeof(numbers[0]);
    for (int i = 0; i < N; i++)
    {
        int steps[MAX_ITER];
        int result = test_collatz_convergence(numbers[i], MAX_ITER, steps);
        if (result > 0)
        {
            for (int j = 0; j < result; j++)
            {
                printf("%d ", steps[j]);
            }
        }
        else
        {
            printf("niepowodzenie operacji");
        }
        printf("\n");
    }
#ifdef DYNAMIC_LOAD
    dlclose(handle);
#endif
    return 0;
}