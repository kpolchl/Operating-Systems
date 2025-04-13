#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>


// to samo tylko popen
int main()
{
    FILE *read_input = popen("cat", "w");
    if (read_input == NULL)
    {
        perror("popen");
        return 1;
    }
    fputs("ABC" , read_input);

    pclose(read_input);
    return 0;
}