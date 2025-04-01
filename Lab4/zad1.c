#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Give only one argument");
        return 1;
    }
    int n = atoi(argv[1]);
    for (int i = 0; i < n; i++)
    {
        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            printf("%d %d\n", (int)getppid(), (int)getpid());
            exit(0);
        }
    }
    for (int i = 0; i < n; i++)
    {
        wait(NULL);
    }
    printf("%d", n);
    return 0;
}