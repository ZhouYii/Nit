#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "const.c"
#include "helpers.c"

/* Blocking function until ack is recieved from the socket descriptor */
char wait_ack(int sock_fd, char* buf, size_t buf_size) {
    memset(buf, '\0', buf_size);
    recv(sock_fd, buf, BUF_SIZE, NO_FLAGS);
    return (strcmp(buf, ACK) == 0);
}
