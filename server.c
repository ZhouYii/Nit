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
#include "socket_helpers.c"
void recv_file(int sock_fd);
void send_file(int sock_fd);
char buf[1+BUF_SIZE];
char cmd_buf[1+BUF_SIZE];

int main(int argc, char** argv) {
    int socket_fd = 0, conn_fd =0;
    struct sockaddr_in server_addr;

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1) 
        err_handler("Failed to get socked file descriptor");

    /* Zero the IPV6 compatibility bits */
    memset(&server_addr, '0', sizeof(server_addr));
    memset(&buf, '\0', sizeof(buf));
    memset(&cmd_buf, '\0', sizeof(cmd_buf));

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
        while(recv(conn_fd, cmd_buf, BUF_SIZE, NO_FLAGS) > 0) 
        {
            printf("Client message: %s\n", cmd_buf);
            send(conn_fd, ACK, strlen(ACK), NO_FLAGS);
            printf("READING BUF"); //LLDB break here. Is the buf being initialized?
            if(recv(conn_fd, buf, BUF_SIZE, NO_FLAGS) <= 0) {
                close(conn_fd);
                break;
            }
            printf("\nBUF CONTENTS %s\n", buf);
            send(conn_fd, ACK, strlen(ACK), NO_FLAGS);
            if(strcmp(cmd_buf, OP_SEND_FILE) == 0)
                recv_file(conn_fd);
            if(strcmp(cmd_buf, OP_GET_FILE) == 0) {
                printf("SEND FILE\n");
                send_file(conn_fd);
            }
            close(conn_fd);
        }
    }
    return 0;
}
void send_file(int conn_fd) {
    strip_newline(buf);
    /* Client sends file name to recieve */
    FILE* f = fopen(buf, "r");
    if(f == NULL) {
        printf("BUF CONTENTS: %s\n", buf);
        printf("FILE OPEN FAIL\n");
        send(conn_fd, OP_DIE, strlen(OP_DIE), NO_FLAGS);
        return;
    }
    wait_ack(conn_fd, cmd_buf, sizeof(cmd_buf));
    printf("SENT RESPONSE\n");
    while(fgets(buf, sizeof(buf), f) != NULL) {
        printf("whileLoop");
        send(conn_fd, buf, strlen(buf), NO_FLAGS);
        printf("SENDING: %s", buf);
    }
    send(conn_fd, OP_EOF, strlen(OP_EOF), NO_FLAGS);
    fclose(f);
    printf("END OF SEND");
}

void recv_file(int conn_fd) {
    /* Client sends encoded file path and permissions */
    char *perm_ptr, *filepath = buf;
    if((perm_ptr = strstr(buf, SEPARATOR)) != NULL) {
        *(perm_ptr) = '\0';
        /* Stick in a NULL and advance a pointer, effectively making two
         * strings. One for file path, one for permissions */
        perm_ptr += strlen(SEPARATOR);
        /* TODO Right here we need to construct the folder path */
        FILE* f = fopen(filepath, "w");
        if(f == NULL)
            err_handler("Failed to open file");
        printf("Preparing to recieve\n");
        while(recv(conn_fd, buf, BUF_SIZE, NO_FLAGS) > 0) {
            if(strncmp(buf, OP_EOF, strlen(OP_EOF)) == 0) {
                fflush(f);
                break;
            }
            fputs(buf, f);
            send(conn_fd, ACK, strlen(ACK), NO_FLAGS);
            /* Clear the buffer for accurate comparisons */
            memset(buf, '\0', sizeof(buf));
        }
        if(strcmp(buf, OP_EOF) == 0)
            printf("File transfer successful\n");
        else
            printf("No EOF?\n");
        /* We flush STDOUT because sometimes display messages are buffered
         * but not printed */
        fflush(NULL);
    }
}
