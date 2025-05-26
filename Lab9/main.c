#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct
{
    double start;
    double end;
    double width;
    int thread_id;
    double *results;
    int *ready;
} thread_data_t;

double function(double x)
{
    return 4.0 / (x * x + 1.0);
}

double calculate_integral(double domain_start, double domain_end, double width)
{
    double result = 0.0;
    int num_intervals = (int)((domain_end - domain_start) / width);

    for (int i = 0; i < num_intervals; i++)
    {
        double x = domain_start + i * width;
        result += function(x) * width;
    }

    double remaining = domain_end - (domain_start + num_intervals * width);
    if (remaining > 0)
    {
        double x = domain_start + num_intervals * width;
        result += function(x) * remaining;
    }

    return result;
}

void *thread_function(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    double result = calculate_integral(data->start, data->end, data->width);

    data->results[data->thread_id] = result;

    data->ready[data->thread_id] = 1;

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf(" %s <rectangle> <num_of_threads>\n", argv[0]);
        return 1;
    }

    double width = atof(argv[1]);
    int k = atoi(argv[2]);

    if (k <= 0)
    {
        printf(" <num_of_threads> > 0\n");
        return 1;
    }

    if (width <= 0)
    {
        printf("<rectangle> > 0\n");
        return 1;
    }

    pthread_t *threads = malloc(k * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(k * sizeof(thread_data_t));
    double *results = malloc(k * sizeof(double));
    int *ready = calloc(k, sizeof(int));

    if (!threads || !thread_data || !results || !ready)
    {
        return 1;
    }

    double domain_start = 0.0;
    double domain_end = 1.0;
    double segment_size = (domain_end - domain_start) / k;

    for (int i = 0; i < k; i++)
    {
        thread_data[i].start = domain_start + i * segment_size;
        thread_data[i].end = thread_data[i].start + segment_size;

        if (i == k - 1)
        {
            thread_data[i].end = domain_end;
        }

        thread_data[i].width = width;
        thread_data[i].thread_id = i;
        thread_data[i].results = results;
        thread_data[i].ready = ready;

        if (pthread_create(&threads[i], NULL, thread_function, &thread_data[i]) != 0)
        {
            return 1;
        }
    }

    int all_ready = 0;
    while (!all_ready)
    {
        all_ready = 1;
        for (int i = 0; i < k; i++)
        {
            if (ready[i] != 1)
            {
                all_ready = 0;
                break;
            }
        }
        if (!all_ready)
        {
            usleep(1000);
        }
    }

    double total_result = 0.0;
    for (int i = 0; i < k; i++)
    {
        total_result += results[i];
    }

    printf("\n=== RESULTS ===\n");
    printf("Calculated integral: %f\n", total_result);
    printf("Theoretical value (π): %f\n", M_PI);
    printf("Absolute error: %f\n", fabs(total_result - M_PI));
    printf("Relative error: %f%%\n", fabs(total_result - M_PI) / M_PI * 100);

    free(threads);
    free(thread_data);
    free(results);
    free(ready);

    return 0;
}