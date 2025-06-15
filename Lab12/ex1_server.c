#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#define UNIX_PATH_MAX 108

int main(){
    int server_fd = -1;
    if ((server_fd = socket(AF_INET, SOCK_STREAM,0))==-1){
        perror("error creating socket");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        perror("error binding socket");
    }
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    // struct sockaddr_in client_addr;
    // socklen_t client_len = sizeof(client_addr);
    // accept(server_fd, (struct sockaddr*)&client_addr, &client_len);


    char buf[64];
    if (recv(server_fd ,buf ,64,0 )== -1){
        perror("Error receiving data");
    }

    printf("Received data: %s\n", buf);

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    return 0;
}
