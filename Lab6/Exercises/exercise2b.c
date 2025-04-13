// C program to implement one side of FIFO
// This side writes first, then reads
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{

    // FIFO file path
    const char * myfifo = "/tmp/myfifo";

    mkfifo(myfifo, 0666);
    int fd = open(myfifo, O_RDONLY);

    char c;
    while (read(fd, &c , 1) >0){
      write(STDOUT_FILENO, &c , 1);
    }
    close(fd);
    printf("E\n");
    return 0;
}