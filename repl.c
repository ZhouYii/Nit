#include "repl.h"

void enter_repl() {
    char buf[_BUF_SIZE];
    printf("Welcome to Nit\n");
    while(scanf("%s", (char*) &buf)) {
        buf[_BUF_SIZE-1] = '\0';
        printf("%s\n", buf);
        process_cmd(&buf);
    }
}

void process_cmd(const char* buf) {
    if(strcmp(buf, "update") == 0) {
        printf("Update source code\n");
        return;
    }
    int cmd_offset = strchr(buf, ' ') + 1;

    /* Magic number describes number of chars to compare */
    if(strcmp(buf+cmd_offset, "add", 3) == 0) {
        printf("Add command\n");
        return;
    }

    if(strcmd(buf+cmd_offset, "delete", 6) == 0) {
        printf("Delete command\n");
        return;
    }

    printf("Not a valid command. Try Nit help\n");

}
