#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    pid_t child_pid;
    printf("PID glownego programu: %d\n", (int)getpid());
    child_pid = fork();
    if (child_pid != 0)
    {
        printf("Proces rodzica: Proces rodzica ma pid:%d\n", (int)getpid());
        printf("Proces rodzica: Proces dziecka ma pid:%d\n", (int)child_pid);
    }
    else
    {
        printf("Proces dziecka: Proces rodzica ma pid:%d\n", (int)getppid());
        printf("Proces dziecka: Proces dziecka ma pid:%d\n", (int)getpid());
    }

    return 0;
}