#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 512
#define NAME_LEN 32
#define PING_INTERVAL 30
#define CLIENT_TIMEOUT 60

typedef struct {
    struct sockaddr_in addr;
    char name[NAME_LEN];
    int active;
    time_t last_alive;
    socklen_t addr_len;
} Client;

Client clients[MAX_CLIENTS];
int server_fd;

void cleanup(int signum) {
    printf("\nShutting down server...\n");
    close(server_fd);
    exit(0);
}

int find_client_by_addr(struct sockaddr_in *addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && 
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

int find_client_by_name(const char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void broadcast(char *msg, struct sockaddr_in *sender_addr) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    char out[BUF_SIZE];
    
    snprintf(out, sizeof(out), "[%s] %s", timestamp, msg);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && 
            !(clients[i].addr.sin_addr.s_addr == sender_addr->sin_addr.s_addr &&
              clients[i].addr.sin_port == sender_addr->sin_port)) {
            sendto(server_fd, out, strlen(out), 0, 
                   (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
        }
    }
}

void send_to_one(char *msg, char *target_name, struct sockaddr_in *sender_addr) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    char out[BUF_SIZE];
    int target_idx = find_client_by_name(target_name);
    int sender_idx = find_client_by_addr(sender_addr);
    
    if (target_idx >= 0) {
        snprintf(out, sizeof(out), "[%s] (private) %s", timestamp, msg);
        sendto(server_fd, out, strlen(out), 0, 
               (struct sockaddr*)&clients[target_idx].addr, clients[target_idx].addr_len);
        
        // Confirmation to sender
        if (sender_idx >= 0) {
            snprintf(out, sizeof(out), "Message sent to %s", target_name);
            sendto(server_fd, out, strlen(out), 0, (struct sockaddr*)sender_addr, sizeof(*sender_addr));
        }
    } else {
        // User not found
        if (sender_idx >= 0) {
            snprintf(out, sizeof(out), "User %s not found", target_name);
            sendto(server_fd, out, strlen(out), 0, (struct sockaddr*)sender_addr, sizeof(*sender_addr));
        }
    }
}

void list_clients(struct sockaddr_in *client_addr) {
    char list[BUF_SIZE] = "Active clients:\n";
    int count = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(list, "- ");
            strcat(list, clients[i].name);
            strcat(list, "\n");
            count++;
        }
    }
    
    if (count == 0) {
        strcpy(list, "No other active clients\n");
    }
    
    sendto(server_fd, list, strlen(list), 0, 
           (struct sockaddr*)client_addr, sizeof(*client_addr));
}

void remove_client(int idx) {
    if (clients[idx].active) {
        printf("Client %s disconnected.\n", clients[idx].name);
        clients[idx].active = 0;
        clients[idx].name[0] = '\0';
        clients[idx].last_alive = 0;
    }
}

int add_client(struct sockaddr_in *addr, const char *name, socklen_t addr_len) {
    // Check if client already exists
    int existing = find_client_by_addr(addr);
    if (existing >= 0) {
        clients[existing].last_alive = time(NULL);
        return existing;
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].addr = *addr;
            clients[i].addr_len = addr_len;
            strncpy(clients[i].name, name, NAME_LEN - 1);
            clients[i].name[NAME_LEN - 1] = '\0';
            clients[i].active = 1;
            clients[i].last_alive = time(NULL);
            return i;
        }
    }
    return -1; // Server full
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, cleanup);
    
    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
        clients[i].name[0] = '\0';
        clients[i].last_alive = 0;
    }

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create UDP socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    printf("UDP Server started on port %d...\n", port);

    char buf[BUF_SIZE];
    time_t last_ping = time(NULL);

    while (1) {
        // Set socket timeout for non-blocking behavior
        struct timeval timeout = {1, 0};
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        memset(buf, 0, BUF_SIZE);
        int n = recvfrom(server_fd, buf, BUF_SIZE - 1, 0, 
                        (struct sockaddr*)&client_addr, &client_len);
        
        if (n > 0) {
            buf[n] = '\0';
            
            // Handle REGISTER command (client connection)
            if (strncmp(buf, "REGISTER ", 9) == 0) {
                char name[NAME_LEN];
                strncpy(name, buf + 9, NAME_LEN - 1);
                name[NAME_LEN - 1] = '\0';
                
                int client_idx = add_client(&client_addr, name, client_len);
                if (client_idx >= 0) {
                    printf("New client registered: %s from %s:%d\n", 
                           name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    
                    char welcome[BUF_SIZE];
                    snprintf(welcome, sizeof(welcome), "Welcome to the chat, %s!", name);
                    sendto(server_fd, welcome, strlen(welcome), 0, 
                           (struct sockaddr*)&client_addr, client_len);
                } else {
                    printf("Server full, rejecting client: %s\n", name);
                    sendto(server_fd, "Server is full. Try again later.", 32, 0,
                           (struct sockaddr*)&client_addr, client_len);
                }
                continue;
            }
            
            // Find client for other commands
            int client_idx = find_client_by_addr(&client_addr);
            if (client_idx < 0) {
                sendto(server_fd, "Not registered. Send REGISTER <name> first.", 43, 0,
                       (struct sockaddr*)&client_addr, client_len);
                continue;
            }
            
            clients[client_idx].last_alive = time(NULL);
            printf("Received from %s: %s\n", clients[client_idx].name, buf);

            if (strncmp(buf, "LIST", 4) == 0) {
                list_clients(&client_addr);
            } else if (strncmp(buf, "2ALL ", 5) == 0) {
                char msg[BUF_SIZE];
                snprintf(msg, sizeof(msg), "%s: %s", clients[client_idx].name, buf + 5);
                broadcast(msg, &client_addr);
            } else if (strncmp(buf, "2ONE ", 5) == 0) {
                char target[NAME_LEN], msg[BUF_SIZE];
                if (sscanf(buf + 5, "%31s %[^\n]", target, msg) == 2) {
                    char full_msg[BUF_SIZE];
                    snprintf(full_msg, sizeof(full_msg), "%s: %s", clients[client_idx].name, msg);
                    send_to_one(full_msg, target, &client_addr);
                } else {
                    sendto(server_fd, "Invalid 2ONE format. Use: 2ONE <username> <message>", 53, 0,
                           (struct sockaddr*)&client_addr, client_len);
                }
            } else if (strncmp(buf, "STOP", 4) == 0) {
                sendto(server_fd, "Goodbye!", 8, 0, 
                       (struct sockaddr*)&client_addr, client_len);
                remove_client(client_idx);
            } else if (strncmp(buf, "ALIVE", 5) == 0) {
                clients[client_idx].last_alive = time(NULL);
            } else {
                sendto(server_fd, "Unknown command. Available: LIST, 2ALL <msg>, 2ONE <user> <msg>, STOP", 69, 0,
                       (struct sockaddr*)&client_addr, client_len);
            }
        }

        // Ping clients periodically and remove inactive ones
        if (time(NULL) - last_ping > PING_INTERVAL) {
            last_ping = time(NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    if (time(NULL) - clients[i].last_alive > CLIENT_TIMEOUT) {
                        printf("Client %s timed out.\n", clients[i].name);
                        remove_client(i);
                    } else {
                        sendto(server_fd, "PING", 4, 0, 
                               (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
                    }
                }
            }
        }
    }

    return 0;
}