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
#include "client_lib.c"

/* Think more about the authentication
#define KEY "password"` */

char send_file(int sock_fd, char* filepath);
char get_file(int sock_fd, char* filepath);
char init_repo();
char add_file(const char* path);
char push();
char commit();
void serialize();
void revert(const int revision);
void set_up_nit();
void delete(char* path);

char recv_buf[BUF_SIZE+1];
int revision = -1;
char cwd[BUF_SIZE];
char* del;

int main(int argc, char** argv)
{
    /* Set up networking stuff */
    recv_buf[BUF_SIZE] = '\0';
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
 
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        err_handler("Cannot create socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    /* Enter interpretation loop */
    if(argc <= 1) 
        err_handler("No commands where given");

    char* cmd = argv[1];
    if(strcmp(cmd, "init") == 0) {
        printf("Welcome to Nit\n");
        set_up_nit();
        return 0;
    } 

    if (init_repo() == FALSE) {
        err_handler("Nit init failed\n");
        return 0;
    }

    if(strcmp(cmd, "push") == 0) {
        printf("Pushing revision %d to server\n", revision);
        push(sock_fd);
        return 0;
    }
    
    if(strcmp(cmd, "update") == 0) {
        printf("Update source code\n");
        return 0;
    }
    
    if(strncmp(cmd, "commit", 6) == 0) {
        printf("Committing revision %d.\n", revision);
        commit();
        serialize();
        return 0;
    } 

    if(argc <= 2) 
        err_handler("Invalid command or command requires more args");

    if(strncmp(cmd, "revert", 6) == 0) {
        revert(atoi(argv[2]));
        serialize();
        return 0;
    } 
   
    if(strncmp(cmd, "add", 3) == 0) {
        add_file(argv[2]);
        serialize();
        return 0;
    }

    if(strncmp(cmd, "delete", 6) == 0)  {
        delete(argv[2]);
        serialize();
        return 0;
    } 

    printf(NO_CMD);
    return 0;
}
void delete(char* path) {
    del = path;
}

/*  Sets up structures for a first-run of nit */
void set_up_nit() {
    FILE* db = fopen(".nitdb", "w");
    char cwd[BUF_SIZE];
    if(getcwd(cwd, BUF_SIZE) == NULL)
        err_handler("Failed to get current working directory");
    fprintf(db, "0\n%s\n", cwd);
    exec("mkdir .nit");
    exec("mkdir .nit/0");
}

/* Check to see if the revision and hashtable are initialized. If not,
 * initilize them from a database file stored on the filesystem */
char init_repo() {
    if(revision < 0) {
        /* Read from the text file and map each thing */
        FILE *load = fopen(".nitdb", "r");
        if(load == NULL) 
            err_handler("Failed to open data store");

        /* Allocated buffer for reading filesys */
        char *line;
        line = malloc(sizeof(char) * (1 + BUF_SIZE));
        if(line == NULL)
            err_handler("Memory allocation failed in initialization");
        line[BUF_SIZE] = '\0';

        /* Read active revision number */
        size_t buf_size = BUF_SIZE;
        if(getline(&line, &buf_size, load) != -1) 
            revision = atoi(line);
        else {
            free(line);
            err_handler("Failed to read revision number!");
        }
        printf("Currently at revision number %d.\n", revision);

        /* Recover CWD */
        if(getline(&line, &buf_size, load) != -1) {
            strip_newline(line);
            strcpy(cwd, line);
        }
        else {
            free(line);
            err_handler("Failed to read CWD.");
        }
        free(line);
    }
    return TRUE;
}


/*  Reverts to an older version.
 *  TODO : more error checking.
 * */
void revert(const int num) {
    if(num < 0 || num >= revision)
        return;
    char exec_buf[BUF_SIZE+1];
    exec_buf[BUF_SIZE] = '\0';
    exec("rm ./*");
    sprintf(exec_buf, "cp ./.nit/%d/* .", num);
    exec(exec_buf);
    revision = num;
}

/* Initialized the nit database to be aware of a new file. Users still have to
 * commit to formally track the file. */
char add_file(const char* path) {
    if(path == NULL)
        err_handler("Added file path is not valid!");
    char t = path[0];
    /* Do not let them add files from parent directories. Potentially security
     * issue if someone sudos and files are copied all over the file system */
    if(t == '~' || t == '/')
        err_handler("File path is not in scope");

    //Add a new entry in the hashtable, populate with the hash checksum.
    if(db_findline(path))
        err_handler("This file is already being tracked by Nit!");
    
    const char* sha512 = hashfile(path);
    char db_buf[BUF_SIZE + 1] = {0};
    db_buf[BUF_SIZE] = '\0';
    sprintf(db_buf, "%s\n%s\n", path, sha512);
    if(!db_writeline(db_buf))
        err_handler("Failed to write to database");
    free((void*)sha512);
    return TRUE;
}

/* Commit : push, but for local. for each key in the hash, see if the sum is still the same. Update
 * revision */
char commit() {
    if(revision < 0) 
        return FALSE;
    size_t buf_size = BUF_SIZE;
    char *buf = malloc(sizeof(char) * (BUF_SIZE + 1));
    char exec_buf[BUF_SIZE + 1];
    buf[BUF_SIZE] = exec_buf[BUF_SIZE] = '\0';
    FILE* db = fopen(".nitdb", "r");
    if(db == NULL) 
        err_handler("Failed to open db from commit()");
    skiplines(db, REPO_OFFSET);
    char is_path = TRUE;
    sprintf(exec_buf, "mkdir ./.nit/%d", revision+1);
    exec(exec_buf);
    while(getline(&buf, &buf_size, db) != -1) {
        if(is_path) {
            strip_newline(buf);
            build_dirs(buf, revision);
            sprintf(exec_buf, "cp %s ./.nit/%d/%s", buf, revision, buf);
            exec(exec_buf);
        }  
        is_path = !is_path;
    }
    sprintf(exec_buf, "cp .nitdb ./.nit/%d/.nitdb", revision);
    exec(exec_buf);
    revision++;
    free(buf);
    return TRUE;
} 

char push(int sock_fd) {
    /* to server */
    /* TODO: Takes a project name */
    /* Sends a bunch of files to server */
    size_t buf_size = BUF_SIZE;
    memset(recv_buf, '\0', sizeof(recv_buf));
    FILE *f = open_db("r");
    skiplines(f, REPO_OFFSET);
    while(getline((char**)&recv_buf, &buf_size, f) != -1) {
        strip_newline(recv_buf);
        printf("Sending file to server : %s\n", recv_buf);
        send_file(sock_fd, recv_buf);
        memset(recv_buf, '\0', sizeof(recv_buf));
    }
    return FALSE;
}


/*  Commit changes to local filesystem.
 */
void serialize() {
    if(revision < 0)
        return; 
    system("cat .nitdb > .nitdb_bak");
    FILE *f = fopen(".nitdb", "w");
    FILE *b = fopen(".nitdb_bak", "r");
    size_t buf_size = BUF_SIZE;
    char *buf = malloc(sizeof(char) * (1 + BUF_SIZE));
    buf[BUF_SIZE] = '\0';
    if(f == NULL || b == NULL) {
        printf("Error serializing database!");
        return;
    }
    /* Serialize revision */
    char* rev = itoc(revision, num_digits(revision));
    if(rev == NULL)
        err_handler("Serializing revision failed");
    printf("Exiting at revision number %s.\n", rev);
    fprintf(f, "%s\n", rev);
    fprintf(f, "%s\n", cwd);
    skiplines(b, REPO_OFFSET);
    while(getline(&buf, &buf_size, b) != -1) {
        strip_newline(buf);
        if(del && (strcmp(del, buf) == 0)) {
            skiplines(b, 1);
        } else
            fprintf(f, "%s\n", buf);
    }
    free(buf);
    free(rev);
}

