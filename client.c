#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "const.c"
#include "helpers.c"
#include "socket_helpers.c"
/* Think more about the authentication
#define KEY "password"` */

char* get_permissions(const char* path);
char wait_ack(int sock_fd, char* buf, size_t buf_size);
char send_file(int sock_fd, char* filepath);
char get_file(int sock_fd, char* filepath);
 
int main(void)
{
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
 
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        err_handler("Cannot create socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
        err_handler("Connection failed");

    printf("RETURN CODE: %d\n", get_file(sock_fd, "test1"));
    return 0; 
}
char get_file(int sock_fd, char* filepath) {
    char recv_buf[BUF_SIZE+1];
    memset(recv_buf, '\0', sizeof(recv_buf));
    send(sock_fd, OP_GET_FILE, strlen(OP_GET_FILE), NO_FLAGS);
    printf("A");
    if(!wait_ack(sock_fd, recv_buf, sizeof(recv_buf))) 
        return -1;
    printf("B");
    send(sock_fd, filepath, strlen(filepath), NO_FLAGS);
    if(!wait_ack(sock_fd, recv_buf, sizeof(recv_buf))) 
        return -1;
    send(sock_fd, ACK, strlen(ACK), NO_FLAGS);
    printf("C");
    /* TODO Make the folder structure */
    FILE* f = fopen(filepath, "w");
    if(f == NULL)
        return -1;
    printf("D");
    memset(recv_buf, '\0', sizeof(recv_buf));
    while(recv(sock_fd, recv_buf, BUF_SIZE, NO_FLAGS) > 0) {
        char* loc = NULL;
        if((loc = strstr(recv_buf, OP_EOF)) != NULL) 
            *(loc) = '\0';
        printf("WRITING TO FILE: %s", recv_buf);
        fputs(recv_buf, f);
        /* Clear the buffer for accurate comparisons */
        memset(recv_buf, '\0', sizeof(recv_buf));
        if(loc)
            break;
    }
    printf("File transfer successful\n");
    fflush(NULL);
    fclose(f);

    return 1;
}

char send_file(int sock_fd, char* filepath) {
    char recv_buf[BUF_SIZE+1];
    memset(recv_buf, '\0', sizeof(recv_buf));
    FILE* f = fopen(filepath, "r");
    if(f == NULL) 
        return -1;

    /* Tell the server we are going to send a file */
    send(sock_fd, OP_SEND_FILE, strlen(OP_SEND_FILE), NO_FLAGS);
    if(!wait_ack(sock_fd, recv_buf, sizeof(recv_buf)))
        return -1;

    /* First send file path and file permissions */
    char* permissions = get_permissions(filepath);
    sprintf(recv_buf, "%s%s%s", filepath, SEPARATOR, permissions);
    send(sock_fd, recv_buf, strlen(recv_buf), NO_FLAGS);
    if(!wait_ack(sock_fd, recv_buf, sizeof(recv_buf)))
        return -1;

    /* Send the file */
    while(fgets(recv_buf, sizeof(recv_buf), f) != NULL) {
        send(sock_fd, recv_buf, strlen(recv_buf), NO_FLAGS);
        if(!wait_ack(sock_fd, recv_buf, sizeof(recv_buf)))
            return -1;
    }
    send(sock_fd, OP_EOF, strlen(OP_EOF), NO_FLAGS);
    free(permissions);
    close(sock_fd);
    return 1;
}


/* Given a line from stat, finds the location of the 9 rwx digits.
 * skips the leading '-' character.
 */
int get_perm_index(const char* buf) {
    if(buf == NULL)
        return -1;
    int num_spaces = 0, index = 0;
    while(num_spaces < 2) {
        if(buf[index] == '\0')
            /* Did not find a permissions string */
            return -1;
        else if(buf[index] == ' ')
            /* Encountered a space */
            num_spaces++;

        index++;
    }
    return ++index;
}

/* Should not use this method outside of get_permissions. There are assumptions
 * about the input buffer format.
 * This function converts three chars into a permissions digit
 */
int calcperm(const char* buf) {
    if(buf == NULL)
        return -1;
    int val = 0;
    if(buf[0] != '-')
        val += 4;
    if(buf[1] != '-')
        val += 2;
    if(buf[2] != '-')
        val += 1;
    return val;
}

char* get_permissions(const char* path) {
    if(path == NULL)
        err_handler("Cannot permissions for invalid file path");
    char buf[BUF_SIZE];

    /* Calculate file permissions and write to local disk */
    sprintf(buf, "stat %s > .tmp", path);
    system(buf);
    FILE* f = fopen(".tmp", "r");
    if(f != NULL) {
        /* Read permissions information */
        if(fgets(buf, BUF_SIZE, f) == NULL)
            err_handler("Cannot read permissions");

        char* p = buf + get_perm_index(buf);
        p[PERMISSION_LEN] = '\0';
        char* permissions = malloc(sizeof(char)*4);
        if(permissions == NULL)
            err_handler("Malloc failed");
        permissions[0] = '0' + calcperm(p);
        permissions[1] = '0' + calcperm(p+3);
        permissions[2] = '0' + calcperm(p+6);
        permissions[3] = '\0';
        printf("Permissions: %s\n", permissions);
        exec("rm .tmp");
        return permissions;
    }
    return NULL;
}
