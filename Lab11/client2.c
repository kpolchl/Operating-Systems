#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#define UNIX_PATH_MAX 108

int main(){
    int fd = -1;
    if ((fd = socket(AF_UNIX, SOCK_DGRAM,0)) == -1){
        perror("error creating socket");
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "\0");

    if (connect(fd , (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        perror("error connecting socket");
    }

    char buf[20];
    int to_send = sprintf(buf, "Hello, World!");

    if (write(fd ,buf ,20 )== -1){
        perror("Error sending message");
    }
    printf("sending: %s\n", buf);

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return 0;
}
