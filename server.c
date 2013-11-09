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

int main(int argc, char** argv) {
    int socket_fd = 0, conn_fd =0;
    struct sockaddr_in server_addr;

    char send_buf[BUF_SIZE + 1];

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1) 
        err_handler("Failed to get socked file descriptor");

    /* Zero the IPV6 compatibility bits */
    memset(&server_addr, '0', sizeof(server_addr));
    memset(&send_buf, '0', sizeof(send_buf));

    server_addr.sin_family = AF_INET;
    /* Standardize endian-ness */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT_NUM);

    if(bind(socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
        err_handler("Failed to bind port");

    if(listen(socket_fd, 10) == -1)
        err_handler("Failed to listen on port");

    while(1) {
        /* Reading from queue of requests */
        conn_fd = accept(socket_fd, (struct sockaddr*) NULL, NULL);
        char recv_buf[BUF_SIZE + 1];
        while(recv(conn_fd, recv_buf, BUF_SIZE, NO_FLAGS) > 0) 
        {
            printf("Client message: %s\n", recv_buf);
            send(conn_fd, "ack", strlen("ack"), NO_FLAGS);
        }
        sleep(1);
    }
        close(conn_fd);
    return 0;
}
