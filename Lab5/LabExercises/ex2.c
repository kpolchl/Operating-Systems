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
            kill(child_pid, SIGUSR1);
            sleep(1);
            send_cnt++;
            printf("send cnt %d\n", send_cnt);
        }
    }
    return 0;
}