// zadanie to samo co wcześniej na potokach nazwanych nie skończone
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    const char * myfifo = "/tmp/myfifo";

    mkfifo(myfifo, 0666);

    int fd = open(myfifo, O_WRONLY );
    write(fd, "ABC\n" , strlen("ABC\n"));
    
    return 0;
}