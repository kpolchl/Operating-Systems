#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 512
#define NAME_LEN 32
#define PING_INTERVAL 30

typedef struct {
    int fd;
    char name[NAME_LEN];
    int active;
    time_t last_alive;
} Client;

Client clients[MAX_CLIENTS];
int server_fd;

void cleanup(int signum) {
    printf("\nShutting down server...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd > 0) {
            close(clients[i].fd);
        }
    }
    close(server_fd);
    exit(0);
}

void broadcast(char *msg, int sender_fd) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    char out[BUF_SIZE];
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd > 0 && clients[i].fd != sender_fd) {
            snprintf(out, sizeof(out), "[%s] %s", timestamp, msg);
            send(clients[i].fd, out, strlen(out), 0);
        }
    }
}

void send_to_one(char *msg, char *target_name, int sender_fd) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    char out[BUF_SIZE];
    int found = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd > 0 && strcmp(clients[i].name, target_name) == 0) {
            snprintf(out, sizeof(out), "[%s] (private) %s", timestamp, msg);
            send(clients[i].fd, out, strlen(out), 0);
            found = 1;
            break;
        }
    }
    
    // Send confirmation to sender
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == sender_fd) {
            if (found) {
                snprintf(out, sizeof(out), "Message sent to %s", target_name);
            } else {
                snprintf(out, sizeof(out), "User %s not found", target_name);
            }
            send(clients[i].fd, out, strlen(out), 0);
            break;
        }
    }
}

void list_clients(int fd) {
    char list[BUF_SIZE] = "Active clients:\n";
    int count = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd > 0) {
            strcat(list, "- ");
            strcat(list, clients[i].name);
            strcat(list, "\n");
            count++;
        }
    }
    
    if (count == 0) {
        strcpy(list, "No other active clients\n");
    }
    
    send(fd, list, strlen(list), 0);
}

void remove_client(int idx) {
    if (clients[idx].fd > 0) {
        printf("Client %s disconnected.\n", clients[idx].name);
        close(clients[idx].fd);
        clients[idx].fd = 0;
        clients[idx].name[0] = '\0';
        clients[idx].active = 0;
        clients[idx].last_alive = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, cleanup);
    
    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = 0;
        clients[i].name[0] = '\0';
        clients[i].active = 0;
        clients[i].last_alive = 0;
    }

    int port = atoi(argv[1]);
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    printf("Server started on port %d...\n", port);

    fd_set read_fds;
    char buf[BUF_SIZE];
    time_t last_ping = time(NULL);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        int max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0) {
                FD_SET(clients[i].fd, &read_fds);
                if (clients[i].fd > max_fd)
                    max_fd = clients[i].fd;
            }
        }

        struct timeval timeout = {1, 0};
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            perror("Select error");
            continue;
        }

        // Handle new connections
        if (FD_ISSET(server_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (new_fd >= 0) {
                memset(buf, 0, BUF_SIZE);
                int name_len = recv(new_fd, buf, NAME_LEN - 1, 0);
                
                if (name_len > 0) {
                    buf[name_len] = '\0';
                    
                    // Find empty slot for new client
                    int slot_found = 0;
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (clients[i].fd == 0) {
                            clients[i].fd = new_fd;
                            strncpy(clients[i].name, buf, NAME_LEN - 1);
                            clients[i].name[NAME_LEN - 1] = '\0';
                            clients[i].active = 1;
                            clients[i].last_alive = time(NULL);
                            printf("New client connected: %s (fd: %d)\n", buf, new_fd);
                            
                            // Send welcome message
                            char welcome[BUF_SIZE];
                            snprintf(welcome, sizeof(welcome), "Welcome to the chat, %s!", buf);
                            send(new_fd, welcome, strlen(welcome), 0);
                            slot_found = 1;
                            break;
                        }
                    }
                    
                    if (!slot_found) {
                        printf("Server full, rejecting client: %s\n", buf);
                        send(new_fd, "Server is full. Try again later.", 32, 0);
                        close(new_fd);
                    }
                } else {
                    close(new_fd);
                }
            }
        }

        // Handle client messages
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0 && FD_ISSET(clients[i].fd, &read_fds)) {
                memset(buf, 0, BUF_SIZE);
                int n = recv(clients[i].fd, buf, BUF_SIZE - 1, 0);
                
                if (n <= 0) {
                    remove_client(i);
                    continue;
                }
                
                buf[n] = '\0';
                clients[i].last_alive = time(NULL);

                printf("Received from %s: %s\n", clients[i].name, buf);

                if (strncmp(buf, "LIST", 4) == 0) {
                    list_clients(clients[i].fd);
                } else if (strncmp(buf, "2ALL ", 5) == 0) {
                    char msg[BUF_SIZE];
                    snprintf(msg, sizeof(msg), "%s: %s", clients[i].name, buf + 5);
                    broadcast(msg, clients[i].fd);
                } else if (strncmp(buf, "2ONE ", 5) == 0) {
                    char target[NAME_LEN], msg[BUF_SIZE];
                    if (sscanf(buf + 5, "%31s %[^\n]", target, msg) == 2) {
                        char full_msg[BUF_SIZE];
                        snprintf(full_msg, sizeof(full_msg), "%s: %s", clients[i].name, msg);
                        send_to_one(full_msg, target, clients[i].fd);
                    } else {
                        send(clients[i].fd, "Invalid 2ONE format. Use: 2ONE <username> <message>", 53, 0);
                    }
                } else if (strncmp(buf, "STOP", 4) == 0) {
                    send(clients[i].fd, "Goodbye!", 8, 0);
                    remove_client(i);
                } else if (strncmp(buf, "ALIVE", 5) == 0) {
                    clients[i].last_alive = time(NULL);
                } else {
                    send(clients[i].fd, "Unknown command. Available: LIST, 2ALL <msg>, 2ONE <user> <msg>, STOP", 69, 0);
                }
            }
        }

        // Ping clients periodically
        if (time(NULL) - last_ping > PING_INTERVAL) {
            last_ping = time(NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd > 0) {
                    if (time(NULL) - clients[i].last_alive > 2 * PING_INTERVAL) {
                        printf("Client %s timed out.\n", clients[i].name);
                        remove_client(i);
                    } else {
                        send(clients[i].fd, "PING", 4, 0);
                    }
                }
            }
        }
    }

    return 0;
}
