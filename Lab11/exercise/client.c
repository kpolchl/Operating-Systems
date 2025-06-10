#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 512
#define NAME_LEN 32

int server_fd;
char name[NAME_LEN];
int running = 1;

void cleanup(int signum) {
    printf("\nDisconnecting from server...\n");
    if (server_fd > 0) {
        send(server_fd, "STOP", 4, 0);
        close(server_fd);
    }
    running = 0;
    exit(0);
}

void *receive_handler(void *arg) {
    char buf[BUF_SIZE];
    
    while (running) {
        memset(buf, 0, BUF_SIZE);
        int len = recv(server_fd, buf, BUF_SIZE - 1, 0);
        
        if (len <= 0) {
            if (running) {
                printf("Connection to server lost.\n");
            }
            break;
        }
        
        buf[len] = '\0';
        
        if (strcmp(buf, "PING") == 0) {
            send(server_fd, "ALIVE", 5, 0);
        } else {
            printf("%s\n", buf);
            fflush(stdout);
        }
    }
    
    return NULL;
}

void print_help() {
    printf("\nAvailable commands:\n");
    printf("  LIST                    - Show all active clients\n");
    printf("  2ALL <message>          - Send message to all clients\n");
    printf("  2ONE <username> <msg>   - Send private message to specific user\n");
    printf("  STOP                    - Disconnect from server\n");
    printf("  HELP                    - Show this help\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <name> <server_ip> <port>\n", argv[0]);
        fprintf(stderr, "Example: %s john 127.0.0.1 8080\n", argv[0]);
        return 1;
    }

    signal(SIGINT, cleanup);
    
    strncpy(name, argv[1], NAME_LEN - 1);
    name[NAME_LEN - 1] = '\0';
    const char *ip = argv[2];
    int port = atoi(argv[3]);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Setup server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", ip);
        close(server_fd);
        return 1;
    }

    // Connect to server
    if (connect(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connection to server failed");
        close(server_fd);
        return 1;
    }

    printf("Connected to server %s:%d as '%s'\n", ip, port, name);
    
    // Send name to server
    send(server_fd, name, strlen(name), 0);

    // Create thread for receiving messages
    pthread_t thread;
    if (pthread_create(&thread, NULL, receive_handler, NULL) != 0) {
        perror("Thread creation failed");
        close(server_fd);
        return 1;
    }

    print_help();
    printf("Enter commands (type HELP for list of commands):\n");

    // Main input loop
    char input[BUF_SIZE];
    while (running && fgets(input, BUF_SIZE, stdin)) {
        // Remove newline
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "HELP") == 0) {
            print_help();
            continue;
        }
        
        if (strcmp(input, "STOP") == 0) {
            send(server_fd, "STOP", 4, 0);
            break;
        }
        
        // Send command to server
        send(server_fd, input, strlen(input), 0);
    }

    running = 0;
    close(server_fd);
    pthread_join(thread, NULL);
    printf("Disconnected from server.\n");
    
    return 0;
}
