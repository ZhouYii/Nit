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

void recv_file();

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
            if(strcmp(recv_buf, OP_SEND_FILE) == 0) {
                printf("Waiting on a file!\n");
                printf("Send ACK 1");
                send(conn_fd, ACK, strlen(ACK), NO_FLAGS);
                if(recv(conn_fd, recv_buf, BUF_SIZE, NO_FLAGS) > 0) {
                    printf("Send ACK 2");
                    send(conn_fd, ACK, strlen(ACK), NO_FLAGS);
                    char *perm_ptr, *filepath = recv_buf;
                    if((perm_ptr = strstr(recv_buf, SEPARATOR)) != NULL) {
                        *(perm_ptr) = '\0';
                        perm_ptr += strlen(SEPARATOR);
                        printf("FILE: %s PERMISSIONS: %s\n", filepath, perm_ptr);
                        /* Right here we need to construct the folder path */
                        FILE* f = fopen(filepath, "w");
                        if(f == NULL)
                            err_handler("Failed to open file");
                        printf("Preparing to recieve\n");
                        while(recv(conn_fd, recv_buf, BUF_SIZE, NO_FLAGS) > 0) {
                            printf("RECV_BUF:%s\n", recv_buf);
                            printf("STRNCMP: %d\n", strncmp(recv_buf, OP_EOF, strlen(OP_EOF)));
                            if(strncmp(recv_buf, OP_EOF, strlen(OP_EOF)) == 0) {
                                printf("ENDHEREHERERERER");
                                fflush(f);
                                break;
                            }
                            printf("Recieving\n");
                            fputs(recv_buf, f);
                            send(conn_fd, ACK, strlen(ACK), NO_FLAGS);
                            /* Clear the buffer for accurate comparisons */
                            memset(recv_buf, '\0', sizeof(recv_buf));
                        }
                        printf("OUR OF WHILE");
                        if(strcmp(recv_buf, OP_EOF) == 0)
                            printf("File transfer successful");
                        else
                            printf("No EOF?");
                    }
                }
            }
            printf("One Done");
        }
        close(conn_fd);
        printf("CLOSED");
    }
    return 0;
}

void recv_file() {
    /* Assume the host first send filename + permissions in the format
     * ///FILENAME///PERMISSIONS. The choice of '/' is because it is not
     * a valid file name character */

    //while 1 until a message is recieved with the correct format

    /* Read a file until the buffer contains an EOF */
}
