#include "collatz.h"

int collatz_conjecture(int input)
{
    if (input % 2 == 0)
    {
        input /= 2;
    }
    else
    {
        input = input * 3 + 1;
    }
    return input;
}
int test_collatz_convergence(int input, int max_iter, int *steps)
{
    for (int i = 0; i < max_iter; i++)
    {
        steps[i] = input;
        if (input == 1)
        {
            return i + 1;
        }
        input = collatz_conjecture(input);
    }
    return 0;
}