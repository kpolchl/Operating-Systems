#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pipe(fd);
    pid_t pid = fork();

    if (pid == 0)
    {                 // dziecko
        close(fd[1]); // zamknięcie końca do zapisu w procesie potomnym

        // char buffer[100];
        // int bytes_read = read(fd[0], buffer, sizeof(buffer)); // odczyt danych z potoku
        // if (bytes_read > 0) {
        //     printf("Dziecko odczytało: %.*s\n", bytes_read, buffer);
        // }

        close(fd[0]);
        
        // _exit(0);
    }
    else
    {                 // rodzic
        close(fd[0]); // zamknięcie końca do odczytu w procesie rodzica
        sleep(1);
        const char *message = "Przykładowa wiadomość od rodzica";
        write(fd[1], message, strlen(message)); // zapis danych do potoku

        close(fd[1]);
        wait(NULL); // oczekiwanie na zakończenie procesu potomnego
    }

    return 0;
}