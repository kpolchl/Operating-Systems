#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pipe(fd); // Tworzymy potok

    pid_t pid = fork();
    if (pid == 0)
    {                 // dziecko
        close(fd[1]); // Zamykamy nieużywany koniec do zapisu

        // Przekierowujemy stdin na potok
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]); // Można zamknąć oryginalny deskryptor

        // Uruchamiamy grep, który będzie czytał z stdin
        execlp("grep", "grep", "Ala", NULL);

        // Jeśli exec się nie powiedzie:
        // perror("execlp");
        _exit(1);
    }
    else
    {                 // rodzic
        close(fd[0]); // Zamykamy nieużywany koniec do odczytu

        const char *lines[] = {
            "Ala ma kota\n",
            "Kot ma Ale\n",
            "Ala lubi psy\n",
            "Psy nie lubią kotów\n",
            "Ala to imię\n"};

        // Zapisujemy dane do potoku
        for (int i = 0; i < 5; i++)
        {
            write(fd[1], lines[i], strlen(lines[i]));
        }

        close(fd[1]); // Koniec zapisu - grep otrzyma EOF
        wait(NULL);   // Czekamy na zakończenie dziecka
    }

    return 0;
}