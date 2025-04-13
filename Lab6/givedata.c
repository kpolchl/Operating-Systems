#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong number of arguments");
        return 1;
    }
    const char *sendfifo = "/tmp/sendfifo";
    const char *resultfifo = "/tmp/resultfifo";
    mkfifo(sendfifo, 0660);
    mkfifo(resultfifo, 0660);
    double a, b;
    a = atof(argv[1]);
    b = atof(argv[2]);
    int fd_in = open(sendfifo, O_WRONLY);
    write(fd_in, &a, sizeof(a));
    write(fd_in, &b, sizeof(b));
    close(fd_in);

    int fd_result = open(resultfifo, O_RDONLY);
    double result;
    read(fd_result, &result, sizeof(result));
    close(fd_result);
    printf("%f\n", result);
    return 0;
}