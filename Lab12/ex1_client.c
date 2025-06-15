#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#define UNIX_PATH_MAX 108

int main(){
    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM,0)) == -1){
        perror("error creating socket");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080); 


    if (connect(fd , (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        perror("error connecting socket");
    }

    char buf[64];
    int to_send = sprintf(buf, "HELLO to proces: %d " , getpid());

    if (send(fd ,buf ,64,0 )== -1){
        perror("Error sending message");
    }
    printf("sending: %s\n", buf);

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return 0;
}
