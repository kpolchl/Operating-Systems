#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_QUEUE_SIZE 10
#define JOB_TEXT_SIZE 10
#define SHM_NAME "/print_system_shm"
#define SEM_MUTEX "/print_system_mutex"
#define SEM_EMPTY "/print_system_empty"
#define SEM_FILLED "/print_system_filled"

typedef struct
{
    char jobs[MAX_QUEUE_SIZE][JOB_TEXT_SIZE + 1];
    int front;
    int rear;
    int count;
} PrintQueue;

sem_t *mutex = NULL;
sem_t *empty_slots = NULL;
sem_t *filled_slots = NULL;
PrintQueue *queue = NULL;
int shm_fd = -1;
pid_t *user_pids = NULL;
pid_t *printer_pids = NULL;
int num_users = 0;
int num_printers = 0;

void cleanup(int signum)
{
    printf("\nCleaning up and terminating...\n");

    if (user_pids != NULL)
    {
        for (int i = 0; i < num_users; i++)
        {
            if (user_pids[i] > 0)
            {
                kill(user_pids[i], SIGTERM);
            }
        }
        free(user_pids);
    }

    if (printer_pids != NULL)
    {
        for (int i = 0; i < num_printers; i++)
        {
            if (printer_pids[i] > 0)
            {
                kill(printer_pids[i], SIGTERM);
            }
        }
        free(printer_pids);
    }

    while (wait(NULL) > 0)
        ;

    if (mutex != NULL)
    {
        sem_close(mutex);
        sem_unlink(SEM_MUTEX);
    }

    if (empty_slots != NULL)
    {
        sem_close(empty_slots);
        sem_unlink(SEM_EMPTY);
    }

    if (filled_slots != NULL)
    {
        sem_close(filled_slots);
        sem_unlink(SEM_FILLED);
    }

    if (queue != NULL)
    {
        munmap(queue, sizeof(PrintQueue));
    }

    if (shm_fd != -1)
    {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }

    exit(0);
}

void generate_random_text(char *text, int length)
{
    for (int i = 0; i < length; i++)
    {
        text[i] = 'a' + (rand() % 26);
    }
    text[length] = '\0';
}

void user_process(int user_id)
{
    srand(time(NULL) ^ (user_id * 1000));

    while (1)
    {
        char job_text[JOB_TEXT_SIZE + 1];
        generate_random_text(job_text, JOB_TEXT_SIZE);

        if (sem_wait(empty_slots) == -1)
        {
            if (errno == EINTR)
            {
                exit(0);
            }
        }

        sem_wait(mutex);

        strcpy(queue->jobs[queue->rear], job_text);
        queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
        queue->count++;

        printf("User %d added job: %s, Queue size: %d\n", user_id, job_text, queue->count);

        sem_post(mutex);

        sem_post(filled_slots);

        sleep(1 + (rand() % 5));
    }
}

void printer_process(int printer_id)
{
    while (1)
    {
        if (sem_wait(filled_slots) == -1)
        {
            if (errno == EINTR)
            {
                exit(0);
            }
        }

        sem_wait(mutex);

        char job_text[JOB_TEXT_SIZE + 1];
        strcpy(job_text, queue->jobs[queue->front]);
        queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
        queue->count--;

        printf("Printer %d received job: %s, Queue size: %d\n", printer_id, job_text, queue->count);

        sem_post(mutex);

        sem_post(empty_slots);

        printf("Printer %d printing: ", printer_id);
        fflush(stdout);

        for (int i = 0; job_text[i] != '\0'; i++)
        {
            printf("%c", job_text[i]);
            fflush(stdout);
            sleep(1);
        }

        printf(" [DONE]\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <num_users> <num_printers>\n", argv[0]);
        return 1;
    }

    num_users = atoi(argv[1]);
    num_printers = atoi(argv[2]);

    if (num_users <= 0 || num_printers <= 0)
    {
        printf("Number of users and printers must be positive integers\n");
        return 1;
    }

    signal(SIGINT, cleanup);

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(shm_fd, sizeof(PrintQueue)) == -1)
    {
        perror("ftruncate");
        cleanup(0);
        return 1;
    }

    queue = mmap(NULL, sizeof(PrintQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (queue == MAP_FAILED)
    {
        perror("mmap");
        cleanup(0);
        return 1;
    }

    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;

    mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED)
    {
        perror("sem_open (mutex)");
        cleanup(0);
        return 1;
    }

    empty_slots = sem_open(SEM_EMPTY, O_CREAT | O_EXCL, 0666, MAX_QUEUE_SIZE);
    if (empty_slots == SEM_FAILED)
    {
        perror("sem_open (empty_slots)");
        cleanup(0);
        return 1;
    }

    filled_slots = sem_open(SEM_FILLED, O_CREAT | O_EXCL, 0666, 0);
    if (filled_slots == SEM_FAILED)
    {
        perror("sem_open (filled_slots)");
        cleanup(0);
        return 1;
    }

    user_pids = (pid_t *)malloc(num_users * sizeof(pid_t));
    printer_pids = (pid_t *)malloc(num_printers * sizeof(pid_t));

    if (user_pids == NULL || printer_pids == NULL)
    {
        perror("malloc");
        cleanup(0);
        return 1;
    }

    for (int i = 0; i < num_users; i++)
    {
        user_pids[i] = fork();

        if (user_pids[i] < 0)
        {
            perror("fork (user)");
            cleanup(0);
            return 1;
        }
        else if (user_pids[i] == 0)
        {
            user_process(i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < num_printers; i++)
    {
        printer_pids[i] = fork();

        if (printer_pids[i] < 0)
        {
            perror("fork (printer)");
            cleanup(0);
            return 1;
        }
        else if (printer_pids[i] == 0)
        {
            printer_process(i + 1);
            exit(0);
        }
    }

    printf("Print System is running with %d users and %d printers.\n", num_users, num_printers);

    while (wait(NULL) > 0)
        ;

    cleanup(0);

    return 0;
}