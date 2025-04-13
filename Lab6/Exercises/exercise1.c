#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

// to było na wejściówce 
int main()
{
    int fd[2];
    pipe(fd);
    pid_t pid = fork();

    if (pid == 0)
    {                 // dziecko
        close(fd[1]); // zamknięcie końca do zapisu w procesie potomnym

        char c;
        while (read(fd[0], &c, 1) > 0) // odczyt danych z potoku
        {
            write(STDOUT_FILENO, &c, 1);
        }
        close(fd[0]); // nie było close
        printf("E\n");
    }
    else
    {                           // rodzic
        close(fd[0]);           // zamknięcie końca do odczytu w procesie rodzica
        write(fd[1], "ABC", 3); // zapis danych do potoku
        close(fd[1]);
        waitpid(pid, NULL, 0); // zapis danych do potoku
    }

    return 0;
}