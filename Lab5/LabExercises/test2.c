#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void au(int sig_no)
{
    printf("Otrzymale signal %d.\n", sig_no);
}
int main()
{
    struct sigaction act;
    act.sa_handler = au;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    while (1)
    {
        printf("Witaj.\n");
        sleep(3);
    }
    return 0;
}