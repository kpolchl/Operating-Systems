#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handle(int signum)
{
    printf("jacie ale sygnal\n");
}
int main()
{
    signal(SIGUSR1, handle);
    raise(SIGUSR1);
    return 0;
}