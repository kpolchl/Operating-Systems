#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* procedura obslugi sygnalu SIGINT */
void obslugaINT(int signum)
{
    printf("Obsluga sygnalu SIGINT\n");
}
int main()
{
    /* zarejestrowanie obslugi sygnalu SIGINT */
    signal(SIGINT, obslugaINT);
    /* nieskonczona petla */
    while (1)
    {
        sleep(100);
    }
}