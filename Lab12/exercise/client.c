#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define BUF_SIZE 512
#define NAME_LEN 32

int server_fd;
char name[NAME_LEN];
int running = 1;
struct sockaddr_in server_addr;
socklen_t server_len;

void cleanup(int signum)
{
    printf("\nDisconnecting from server...\n");
    if (server_fd > 0)
    {
        sendto(server_fd, "STOP", 4, 0, (struct sockaddr *)&server_addr, server_len);
        close(server_fd);
    }
    running = 0;
    exit(0);
}

void *receive_handler(void *arg)
{
    char buf[BUF_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (running)
    {
        memset(buf, 0, BUF_SIZE);

        // Set socket timeout for non-blocking behavior
        struct timeval timeout = {1, 0};
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        int len = recvfrom(server_fd, buf, BUF_SIZE - 1, 0,
                           (struct sockaddr *)&from_addr, &from_len);

        if (len > 0)
        {
            buf[len] = '\0';

            if (strcmp(buf, "PING") == 0)
            {
                // Respond to ping with ALIVE
                sendto(server_fd, "ALIVE", 5, 0,
                       (struct sockaddr *)&server_addr, server_len);
            }
            else
            {
                printf("%s\n", buf);
                fflush(stdout);
            }
        }
        else if (len < 0)
        {
            // Check if it's a timeout (not an error in UDP context)
            continue;
        }
    }
    return NULL;
}

void print_help()
{
    printf("\nAvailable commands:\n");
    printf(" LIST - Show all active clients\n");
    printf(" 2ALL <message> - Send message to all clients\n");
    printf(" 2ONE <username> <msg> - Send private message to specific user\n");
    printf(" STOP - Disconnect from server\n");
    printf(" HELP - Show this help\n\n");
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <name> <server_ip> <port>\n", argv[0]);
        fprintf(stderr, "Example: %s john 127.0.0.1 8080\n", argv[0]);
        return 1;
    }

    signal(SIGINT, cleanup);

    strncpy(name, argv[1], NAME_LEN - 1);
    name[NAME_LEN - 1] = '\0';
    const char *ip = argv[2];
    int port = atoi(argv[3]);

    // Create UDP socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Setup server address
    server_len = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid IP address: %s\n", ip);
        close(server_fd);
        return 1;
    }

    printf("Connecting to UDP server %s:%d as '%s'\n", ip, port, name);

    // Register with server
    char register_msg[BUF_SIZE];
    snprintf(register_msg, sizeof(register_msg), "REGISTER %s", name);

    if (sendto(server_fd, register_msg, strlen(register_msg), 0,
               (struct sockaddr *)&server_addr, server_len) < 0)
    {
        perror("Registration failed");
        close(server_fd);
        return 1;
    }

    // Wait for registration response
    char response[BUF_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    struct timeval timeout = {5, 0}; // 5 second timeout
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    int len = recvfrom(server_fd, response, BUF_SIZE - 1, 0,
                       (struct sockaddr *)&from_addr, &from_len);

    if (len <= 0)
    {
        fprintf(stderr, "No response from server or registration failed\n");
        close(server_fd);
        return 1;
    }

    response[len] = '\0';
    printf("%s\n", response);

    if (strstr(response, "Server is full") != NULL)
    {
        close(server_fd);
        return 1;
    }

    // Create thread for receiving messages
    pthread_t thread;
    if (pthread_create(&thread, NULL, receive_handler, NULL) != 0)
    {
        perror("Thread creation failed");
        close(server_fd);
        return 1;
    }

    print_help();
    printf("Enter commands (type HELP for list of commands):\n");

    // Main input loop
    char input[BUF_SIZE];
    while (running && fgets(input, BUF_SIZE, stdin))
    {
        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        if (strcmp(input, "HELP") == 0)
        {
            print_help();
            continue;
        }

        if (strcmp(input, "STOP") == 0)
        {
            sendto(server_fd, "STOP", 4, 0,
                   (struct sockaddr *)&server_addr, server_len);
            break;
        }

        // Send command to server
        sendto(server_fd, input, strlen(input), 0,
               (struct sockaddr *)&server_addr, server_len);
    }

    running = 0;
    close(server_fd);
    pthread_join(thread, NULL);
    printf("Disconnected from server.\n");

    return 0;
}