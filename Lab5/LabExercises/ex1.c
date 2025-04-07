#include <stdio.h>
#include <signal.h>

int main()
{
    signal(SIGKILL, SIG_IGN);
    raise(SIGKILL);
    printf("Hello");
    return 0;
}