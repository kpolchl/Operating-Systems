#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    // Otwieramy potok do programu grep w trybie zapisu ("w")
    FILE *grep_input = popen("grep Ala", "w");
    if (grep_input == NULL)
    {
        perror("popen");
        return EXIT_FAILURE;
    }

    // Dane do przesłania do grep-a
    const char *lines[] = {
        "Ala ma kota\n",
        "Kot ma Ale\n",
        "Ala lubi psy\n",
        "Psy nie lubią kotów\n",
        "Ala to imię\n",
        NULL // znacznik końca
    };

    // Zapisujemy dane do potoku grep-a
    for (int i = 0; lines[i] != NULL; i++)
    {
        if (fputs(lines[i], grep_input) == EOF)
        {
            perror("fputs");
            pclose(grep_input);
            return EXIT_FAILURE;
        }
    }

    // Zamykamy potok - to spowoduje wysłanie EOF do grep-a
    int status = pclose(grep_input);
    if (status == -1)
    {
        perror("pclose");
        return EXIT_FAILURE;
    }
    else if (WIFEXITED(status))
    {
        printf("Grep zakończył się z kodem: %d\n", WEXITSTATUS(status));
    }

    return EXIT_SUCCESS;
}