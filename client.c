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
 
int main(void)
{
    free(get_permissions("helpers.c"));
    /*
    int sock_fd = 0,n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;
 
    memset(recvBuff, '0' ,sizeof(recvBuff));
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    send(sock_fd, "hi", strlen("hi"), NO_FLAGS);
    char in_buf[BUF_SIZE+1];
    int len;
    while((len = read((int)stdin, in_buf, BUF_SIZE)) > 0) {
        printf("Transmitting message : %s\n", in_buf);
        send(sock_fd, in_buf, len, NO_FLAGS);
        n = recv(sock_fd, recvBuff, sizeof(recvBuff)-1, NO_FLAGS);
        if(n > 0) {
            recvBuff[n] = 0;
            printf("RECIEVED : %s\n", recvBuff);
        }
    }

    if(n < 0)
      printf("\n Read Error \n");
    return 0; */
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
