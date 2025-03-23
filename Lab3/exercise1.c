#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

int main()
{

    double time_spent = 0.0;
    clock_t begin = clock();

    char blok[1024];
    int lorem;
    lorem = open("large.txt", O_RDONLY);
    lseek(lorem, 1003, SEEK_CUR);
    while((read(lorem,blok,sizeof(blok)))>0)
        printf("%c", blok);

    close(lorem);
    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("The elapsed time is %f seconds", time_spent);
}