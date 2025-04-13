#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

double function(double x)
{
    return 4.0 / (x * x + 1.0);
}

double calculate(double domain_start, double domain_end, double width)
{
    double result = 0.0;
    int num_intervals = (int)((domain_end - domain_start) / width);

    // Add the contribution from each interval
    for (int i = 0; i < num_intervals; i++)
    {
        double x = domain_start + i * width;
        result += function(x) * width;
    }

    // Handle the remaining piece if the domain isn't evenly divisible by width
    double remaining = domain_end - (domain_start + num_intervals * width);
    if (remaining > 0)
    {
        double x = domain_start + num_intervals * width;
        result += function(x) * remaining;
    }

    return result;
}

void fork_calculate(double domain_start, double domain_end, double width, int k)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int fd[2];
    pipe(fd);

    // Calculate size of each segment
    double segment_size = (domain_end - domain_start) / k;

    for (int i = 0; i < k; i++)
    {
        double segment_start = domain_start + i * segment_size;
        double segment_end = segment_start + segment_size;

        if (i == k - 1)
        {
            segment_end = domain_end;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            close(fd[0]);
            double result = calculate(segment_start, segment_end, width);

            write(fd[1], &result, sizeof(result));
            close(fd[1]);
            exit(0);
        }
    }
    close(fd[1]);
    double partial_result = 0.0;
    double total = 0.0;
    for (int i = 0; i < k; i++)
    {
        wait(NULL);
        read(fd[0], &partial_result, sizeof(partial_result));
        total += partial_result;
    }
    close(fd[0]);

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0;

    printf("For k=%d result=%f and elapsed_time=%f\n", k,total,elapsed);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Give 3 arguments");
        return 1;
    }

    double width = atof(argv[1]);
    int n = atoi(argv[2]);

    for (int i = 0; i <= n; i++)
    {
        fork_calculate(0, 1, width, i);
    }
}