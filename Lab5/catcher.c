#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int signals_received = 0;
volatile sig_atomic_t current_mode = 0;
volatile sig_atomic_t sender_pid = 0;

void handleCtrC(int signum)
{
    printf("CTR+C pressed\n");
}

void handle(int signum, siginfo_t *info, void *context)
{
    signals_received++;

    sender_pid = info->si_pid;
    current_mode = info->si_value.sival_int;

    kill(sender_pid, SIGUSR1);

    switch (current_mode)
    {
    case 1:
        printf("Signal %d received\n", signals_received);
        break;
    case 3:
        signal(SIGINT, SIG_IGN);
        printf("CTR+C ignored\n");
        break;
    case 4:
        struct sigaction ctrlc_act;
        ctrlc_act.sa_handler = handleCtrC;
        sigemptyset(&ctrlc_act.sa_mask);
        ctrlc_act.sa_flags = SA_RESTART;
        sigaction(SIGINT, &ctrlc_act, NULL);
        printf("CTR+C handler set\n");
        break;
    case 5:
        printf("Program terminated by sender\n");
        exit(0);
    }
}

int main()
{
    printf("Catcher PID %d\n", getpid());

    struct sigaction act;
    act.sa_sigaction = handle;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);

    int counter = 0;
    while (1)
    {
        if (current_mode == 2)
        {
            printf("%d\n", counter++);
            sleep(1);
        }
        else
        {
            pause();
        }
    }
    return 0;
}