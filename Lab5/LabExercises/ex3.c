#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int rec_cnt = 0;
int send_cnt = 0;
void handle(int signum)
{
    rec_cnt++;
    printf("Reacivied signals %d\n", rec_cnt);
}
int main()
{
    union sigval value;
    value.sival_int = 0;

    signal(SIGUSR1, handle);
    pid_t child_pid = fork();
    if (child_pid == 0)
    {
        while (1)
        {
            pause();
        }
    }
    else
    {
        while (1)
        {
            sigqueue(child_pid,SIGUSR1,value);
            sleep(1);
            printf("send cnt %d\n", value);
        }
    }
    return 0;
}