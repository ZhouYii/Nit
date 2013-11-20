#ifndef NIT_CONST
#define BUF_SIZE 1024
#define REPO_OFFSET 2
#define SHA512_SIZE 128
#define NO_CMD "Not a valid command. Try Nit help.\n"
#define PORT_NUM 5000
#define NO_FLAGS (int) 0
#define PERMISSION_LEN 9
#define ACK "_ACK"
#define SEPARATOR "////"
/* Server operation constants. Basically so I can incorporate a magic numbers
 * to reduce chance of misunderstood server messages 
 */
#define OP_SEND_FILE "**OP_SEND_FILE@@"
#define OP_GET_FILE "99OP_GET_FILE11"
#define OP_DIE "##OP_DIE&&"
#define OP_EOF "EOF_EOF_EOF"
#define OP_READY "awcfuk.READY"
#endif
