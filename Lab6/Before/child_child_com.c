#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pipe(fd); // Utworzenie potoku

    if (fork() == 0)
    {                 // dziecko 1 - pisarz
        close(fd[0]); // Zamknięcie nieużywanego końca do odczytu

        const char *message = "Wiadomość od pisarza";
        write(fd[1], message, strlen(message) + 1); // +1 dla '\0'
        printf("Pisarz wysłał: %s\n", message);

        close(fd[1]);
        return 0;
    }
    else if (fork() == 0)
    {                 // dziecko 2 - czytelnik
        close(fd[1]); // Zamknięcie nieużywanego końca do zapisu

        char buffer[100];
        int bytes_read = read(fd[0], buffer, sizeof(buffer));
        if (bytes_read > 0)
        {
            printf("Czytelnik odebrał: %s\n", buffer);
        }

        close(fd[0]);
        return 0;
    }
    else
    {                 // rodzic
        close(fd[0]); // Rodzic zamyka oba końce
        close(fd[1]);

        // Oczekiwanie na oba procesy potomne
        wait(NULL);
        wait(NULL);
        printf("Rodzic zakończył pracę.\n");
    }

    return 0;
}