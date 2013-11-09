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
 
int main(void)
{
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
    /*
    while((n = read(sock_fd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
            printf("\n Error : Fputs error");
        printf("\n");
    } */
    
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
    return 0;
}
