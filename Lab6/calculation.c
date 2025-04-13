#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

double function(double x)
{
    return 4 / (x * x + 1);
}

int main()
{

    const char *sendfifo = "/tmp/sendfifo";
    const char *resultfifo = "/tmp/resultfifo";
    mkfifo(sendfifo, 0660);
    mkfifo(resultfifo, 0660);

    int fd_in = open(sendfifo, O_RDONLY);
    double a, b;
    read(fd_in, &a, sizeof(a));
    read(fd_in, &b, sizeof(b));

    close(fd_in);

    int accuracy = 1000;
    double width = (b - a) / accuracy;
    double result = 0.0;
    for (int i = 0; i < accuracy; i++)
    {
        double x = a + i * width;
        result += function(x) * width;
    }

    int fd_result = open(resultfifo, O_WRONLY);
    write(fd_result, &result, sizeof(result));
    close(fd_result);
    return 0;
}