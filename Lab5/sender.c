#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

void confirmation_handler(int sig)
{
    printf("Confirmation signal send to catcher. Exiting...\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Max 3 arg");
        return 1;
    }
    pid_t catcher_pid = atoi(argv[1]);
    int signal_type = atoi(argv[2]);

    if (signal_type < 1 || signal_type > 5)
    {
        printf("Signal type must be between 1 and 5");
        return 1;
    }

    struct sigaction act;
    act.sa_handler = confirmation_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    union sigval value;
    value.sival_int = signal_type;

    if (sigqueue(catcher_pid, SIGUSR1, value) != 0)
    {
        perror("error during sigqueue");
        return 1;
    }
    pause();

    return 0;
}
