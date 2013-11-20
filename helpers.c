#ifndef HELPERS_C
#define HELPERS_C
#include "const.c"
#include <glib.h>
#include "stdlib.h"
#include "string.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Calculates SHA512 has for a file specified by filepath.
 * mallocs so calling function must remember to free
 * The buffer returned is of size SHA512_SIZE and null terminated*/
const char* hashfile(const char* filepath) {
    size_t new_len;
    if(filepath == NULL)
        return FALSE;
    FILE* f = fopen(filepath, "r");
    if(f == NULL)
        return FALSE;
    GChecksum *chksum = g_checksum_new(G_CHECKSUM_SHA512);
    char buf[BUF_SIZE + 1] = {0};
    do {
        new_len = fread((void*) buf, sizeof(char), BUF_SIZE, f);
        g_checksum_update(chksum, (void*) buf, new_len);
    } while(new_len != 0);

    char* hash_str = malloc(sizeof(char) * (SHA512_SIZE+1));
    hash_str[SHA512_SIZE] = '\0';
    strcpy(hash_str, g_checksum_get_string(chksum));
    g_checksum_free(chksum);
    return hash_str;
}

/*  Does a byte-wise copy of a file at src_path to a new file a dest_path.
 *  If the dest file does not exist, then it is created. 
 *  Fails if the src_path file does not exist.
 */
char copy_file(char* src_path, char* dest_path) {
    if(src_path == NULL || dest_path == NULL)
        return FALSE;

    FILE *src = fopen(src_path, "r");
    if(src == NULL) {
        printf("Failed to open src file\n");
        return FALSE;
    }
    FILE *dest = fopen(dest_path, "w");
    if(dest == NULL) {
        printf("Failed to create destination file\n");
        return FALSE;
    }

    char read;
    while((read = getc(src)) != EOF) 
        putc(read, dest);

    return TRUE;
}

void err_handler(const char* msg) {
    if(msg)
        printf("%s\n", msg);
    exit(1);
}

/* When reading serialized data back from filesystem, the newly inserted
 * newline characters corrupt data. This function removed the last newline char
 * from strings */
void strip_newline(char *buf) {
    if(buf == NULL)
        return;
    int i = 0;
    while(buf[i] != '\0')
        i++;
    if(buf[i-1] == '\n')
        buf[--i] = '\0';
}

/* Scans the nit database for a matching line. Returns booleans (0 or 1) 
 */
char db_findline(const char* line) {
    char *buf = malloc(sizeof(char) * (BUF_SIZE + 1));
    if(line == NULL || buf == NULL)
        return FALSE;
    int len; 
    size_t buf_size = BUF_SIZE;
    buf[BUF_SIZE] = '\0';
    FILE *db = fopen(".nitdb", "r");
    if(db == NULL)
        err_handler("Failed to open file in db_findline");
    while((len = getline(&buf, &buf_size, db)) != -1) {
        strip_newline(buf);
        if(strcmp(line, buf) == 0) {
            free(buf);
            return TRUE;
        }
    }
    free(buf);
    return FALSE;
}

/* Opens Nit's database. Returns a file pointer.
 * The open mode is r/w/rw/r+/w+... etc
 */
FILE* open_db(char* mode) {
    /* No malloc because pass by value should be okay */
    FILE *f = fopen(".nitdb", mode);
    if(f == NULL)
        err_handler("Failed to open database");
    return f;
}

/*  Appends a line into the nit database file using append mode 
 */
char db_writeline(const char* line) {
    FILE *db = fopen(".nitdb", "a");
    if(db == NULL)
        err_handler("Failed to open file in db_writeline");
    if(fprintf(db, "%s", line) < 0)
        err_handler("Failed to write to database");
    fflush(db);
    return TRUE;
}

/* Returns the size needed for a buffer to store an ASCII representation of an
 * integer. 
 */
int num_digits(int n) {
    int i = 0;
    do {
        i++;
        n /= 10;
    } while(n != 0);
    return i;
}

/*  Returns a buffer holding the ASCII contents after val is converted.
 *  The returned buffer is allocated on heap.
 *  Only works with positive numbers. Negative revision numbers are wrong
 *  anyways.
 */
char* itoc(int val, int digits) {
    char *buf = malloc(sizeof(char) * (digits + 1));
    if(buf == NULL) 
        return NULL;
    buf[digits] = '\0';
    for(int i = (digits-1); i >= 0; i--) {
        buf[i] = '0' + (val % 10);
        val /= 10;
    }
    return buf;
}

/*  Skips 'lines' number of lines in the FILE input stream 
 */
void skiplines(FILE* f, int lines) {
    if(f == NULL)
        return;
    char c;
    while(lines > 0 && (c = fgetc(f)) != EOF) 
        if(c == '\n')
            lines--;
}

/* Silent interface to system shell */
void exec(const char *cmd) {
    if(cmd == NULL)
        return;
    int len = strlen(cmd);
    char * buffer = malloc(sizeof(char) * (len + 18));
    if(buffer == NULL) 
        return;
    strncpy(buffer, cmd, len);
    /* Commands writing probably will fail because of this line */
    strcpy((buffer + len), " > /dev/null 2>&1");
    system(buffer);
    free(buffer);
}

/* Matching file system tree in the store folder */
void build_dirs(char* path, int revision) {
    if(path == NULL)
        err_handler("build_dirs took a NULL ptr");
    int i = 0;
    char* buf = malloc(sizeof(char) * (1 + BUF_SIZE));
    buf[BUF_SIZE] = '\0';
    while(path[i] != '\0') {
        if(path[i] == '/') {
            path[i] = '\0';
            sprintf(buf, "mkdir ./.nit/%d/%s", revision, path);
            path[i] = '/';
            exec(buf);
        }
        i++;
    }
    free(buf);
}

/* Blocking function until ack is recieved from the socket descriptor */
char wait_ack(int sock_fd, char* buf, size_t buf_size) {
    memset(buf, '\0', buf_size);
    recv(sock_fd, buf, BUF_SIZE, NO_FLAGS);
    return (strcmp(buf, ACK) == 0);
} 
#endif
