#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#define MSG_SIZE 1024

mqd_t client_queue;
char client_queue_name[64];

void cleanup(int signum) {
    mq_close(client_queue);
    mq_unlink(client_queue_name);
    printf("\nClient exiting. Queue removed.\n");
    exit(0);
}

void* receiver_thread(void* arg) {
    char buffer[MSG_SIZE];
    while (1) {
        ssize_t bytes_read = mq_receive(client_queue, buffer, sizeof(buffer), NULL);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);
        }
    }
    return NULL;
}

int main() {
    signal(SIGINT, cleanup);

    pid_t pid = getpid();
    sprintf(client_queue_name, "/client_queue_%d", pid);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;

    client_queue = mq_open(client_queue_name, O_CREAT | O_RDONLY, 0644, &attr);
    if (client_queue == -1) {
        perror("Error creating client queue");
        exit(EXIT_FAILURE);
    }

    mqd_t server_queue = mq_open("/server_queue", O_WRONLY);
    if (server_queue == -1) {
        perror("Error opening server queue");
        exit(EXIT_FAILURE);
    }

    char init_msg[MSG_SIZE];
    sprintf(init_msg, "INIT %s", client_queue_name);
    mq_send(server_queue, init_msg, strlen(init_msg) + 1, 0);

    char buffer[MSG_SIZE];
    ssize_t bytes_read = mq_receive(client_queue, buffer, sizeof(buffer), NULL);
    if (bytes_read == -1) {
        perror("Error receiving ID");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0';

    int client_id;
    sscanf(buffer, "ID %d", &client_id);
    printf("Registered with server. Assigned ID: %d\n", client_id);

    pthread_t receiver;
    pthread_create(&receiver, NULL, receiver_thread, NULL);

    char msg[MSG_SIZE];
    while (1) {
        if (fgets(msg, sizeof(msg), stdin) != NULL) {
            msg[strcspn(msg, "\n")] = '\0'; // remove newline
            char send_msg[MSG_SIZE];
            sprintf(send_msg, "%d %s", client_id, msg);
            mq_send(server_queue, send_msg, strlen(send_msg) + 1, 0);
        }
    }

    mq_close(server_queue);
    cleanup(0);
    return 0;
}
