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

/* Think more about the authentication
#define KEY "password"` */

char* get_permissions(const char* path);
char wait_ack(int sock_fd, char* buf);
char send_file(int sock_fd, char* filepath);
 
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

    send(sock_fd, "A", strlen("A"), NO_FLAGS);
    if(wait_ack(sock_fd, recv_buf))
        printf("works");
    send(sock_fd, "A", strlen("A"), NO_FLAGS);
    if(wait_ack(sock_fd, recv_buf))
        printf("works");
    send(sock_fd, "A", strlen("A"), NO_FLAGS);
    if(wait_ack(sock_fd, recv_buf))
        printf("works");
    send(sock_fd, "A", strlen("A"), NO_FLAGS);
    if(wait_ack(sock_fd, recv_buf))
        printf("works");
    return 0; 
}

char send_file(int sock_fd, char* filepath) {
    char recv_buf[BUF_SIZE+1];
    memset(recv_buf, '\0', sizeof(recv_buf));
    FILE* f = fopen(filepath);
}

char wait_ack(int sock_fd, char* buf) {
    recv(sock_fd, buf, BUF_SIZE, NO_FLAGS);
    return (strcmp(buf, ACK) == 0);
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
