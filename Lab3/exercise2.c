#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
    char c;
    int lorem;
    lorem = open("lorem.txt", O_RDONLY);
    int offset = 10;
    while (read(lorem, &c, 1) == 1)
        printf("%c",c);

    close(lorem);
}