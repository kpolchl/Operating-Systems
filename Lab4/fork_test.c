#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

// kod tworzy n procesów każdy wypisuje
int main()
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        printf("%d\n", getpid());
        execl("/bin/ls", "ls", "-ld", NULL);
        exit(0);
    }
    exit(0);
}