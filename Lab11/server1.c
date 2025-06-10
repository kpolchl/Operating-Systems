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
    if ((fd = socket(AF_UNIX, SOCK_DGRAM,0))==-1){
        perror("error creating socket");
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';

    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        perror("error binding socket");
    }
    char buf[20];
    if (read(fd ,buf ,20 )== -1){
        perror("Error receiving data");
    }
    printf("Received data: %s\n", buf);

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return 0;
}
