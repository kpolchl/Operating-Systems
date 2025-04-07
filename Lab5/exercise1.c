#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void handle(int signum)
{
    printf("Signal received\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Give only two argumants\n");
        return 1;
    }
    if (strcmp("none", argv[1]) == 0)
    {
        signal(SIGUSR1, SIG_DFL);
    }
    else if (strcmp("ignore", argv[1]) == 0)
    {
        signal(SIGUSR1, SIG_IGN);
    }
    else if (strcmp("handler", argv[1]) == 0)
    {
        signal(SIGUSR1, &handle);
    }
    else if (strcmp("mask", argv[1]) == 0)
    {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    raise(SIGUSR1);

    if (strcmp("mask", argv[1]) == 0)
    {
        sigset_t pending;
        sigpending(&pending);
        if (sigismember(&pending, SIGUSR1))
        {
            printf("Signal is pending\n");
        }
        else
        {
            printf("Signal is not pending\n");
        }
    }
    return 0;
}