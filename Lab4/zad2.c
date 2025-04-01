#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int global = 0;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Give only one argument\n");
        return 1;
    }
    printf("Current programme name %s\n", argv[0]);

    int local = 0;
    pid_t childp = fork();
    if (childp == 0)
    {
        printf("child process\n");
        global++;
        local++;
        printf("child pid = %d parent pid = %d\n", (int)getpid(), (int)getppid());
        printf("child's local = %d, child's global = %d\n", local, global);
        execl("bin/ls", "ls", argv[1], NULL);
    }
    else
    {
        int child_exit_code;
        printf("parent process\n");
        printf("parent pid = %d, child pid = %d\n", (int)getpid(), (int)getppid());
        wait(&child_exit_code);
        printf("Child exit code: %d\n", WEXITSTATUS(child_exit_code));
        printf("Parent's local = %d, parent's global = %d\n", local, global);
        return 0;
    }
    return 0;
}