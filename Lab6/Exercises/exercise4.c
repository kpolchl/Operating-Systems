// ps aux | wc -l zastąpi symbol przekierowania | wywołaniem popoen

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    FILE *pipe, *wc;
    char buffer[1024];
    pipe = popen("ps aux", "r");
    if (pipe == NULL)
    {
        perror("ps aut failed");
        return 1;
    }
    wc = popen("wc -l", "w");
    if (wc == NULL)
    {
        perror("wc -l failed");
        return 1;
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        fputs(buffer, wc);
    }
    pclose(pipe);
    pclose(wc);
    return 0;
}