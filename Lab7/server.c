#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define MSG_SIZE 1024

typedef struct
{
    int id;
    char queue_name[64];
    mqd_t queue;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

mqd_t server_queue;

void cleanup(int signum)
{
    mq_close(server_queue);
    mq_unlink("/server_queue");
    printf("\nServer shutting down. Queue removed.\n");
    exit(0);
}

void broadcast_message(const char *msg, int from_id)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].id != from_id)
        {
            mq_send(clients[i].queue, msg, strlen(msg) + 1, 0);
        }
    }
}

int main()
{
    signal(SIGINT, cleanup);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;

    server_queue = mq_open("/server_queue", O_CREAT | O_RDONLY, 0644, &attr);
    if (server_queue == -1)
    {
        perror("Error creating server queue");
        exit(EXIT_FAILURE);
    }
    printf("Server started. Queue created.\n");

    char buffer[MSG_SIZE];

    while (1)
    {
        ssize_t bytes_read = mq_receive(server_queue, buffer, sizeof(buffer), NULL);
        if (bytes_read == -1)
        {
            perror("Error receiving message");
            continue;
        }

        buffer[bytes_read] = '\0'; // null-terminate
        printf("Received: %s\n", buffer);

        if (strncmp(buffer, "INIT ", 5) == 0)
        {
            if (client_count >= MAX_CLIENTS)
            {
                printf("Max clients reached.\n");
                continue;
            }
            char client_queue_name[64];
            sscanf(buffer + 5, "%s", client_queue_name);

            mqd_t client_queue = mq_open(client_queue_name, O_WRONLY);
            if (client_queue == -1)
            {
                perror("Error opening client queue");
                continue;
            }

            clients[client_count].id = client_count + 1;
            strcpy(clients[client_count].queue_name, client_queue_name);
            clients[client_count].queue = client_queue;

            char id_msg[32];
            sprintf(id_msg, "ID %d", clients[client_count].id);
            mq_send(client_queue, id_msg, strlen(id_msg) + 1, 0);

            printf("Client %d registered with queue %s\n", clients[client_count].id, client_queue_name);
            client_count++;
        }
        else
        {
            int client_id;
            char message[MSG_SIZE];
            sscanf(buffer, "%d %[^\n]", &client_id, message);
            printf("Client %d says: %s\n", client_id, message);
            broadcast_message(buffer, client_id);
        }
    }

    mq_close(server_queue);
    mq_unlink("/server_queue");
    return 0;
}
