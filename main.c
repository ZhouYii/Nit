#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include "helpers.c"
#include "const.c"


/* Populate current directory with required files for starting a repo. 
 * Returns 0 (False) on failure and 1 (True) on success. */
char init_repo();
char add_file(const char* path);
char push();
char commit();
void serialize();
void revert(const int revision);
void set_up_nit();
void delete(char* path);
void build_dirs(char* path);

int revision = -1;
char cwd[BUF_SIZE];
char* del;

int main(int argc, char** argv) {
    if(argc <= 1) {
        printf(NO_CMD);
        return 0;
    }
    char* cmd = argv[1];
    if(strcmp(cmd, "init") == 0) 
    {
        printf("Welcome to Nit\n");
        set_up_nit();
    } 
    else if (init_repo() == FALSE) 
    {
        err_handler("Nit init failed\n");
    } 
    else if(strcmp(cmd, "update") == 0) 
    {
        printf("Update source code\n");
    } 
    else if(strncmp(cmd, "commit", 6) == 0) 
    {
        printf("Committing revision %d.\n", revision);
        commit();
        serialize();
    } 
    else if(argc <= 2) 
    {
        /* Commands from this point on require an argument */
        printf("Command need more arguments. Try Nit help. \n");
    } 
    else if(strncmp(cmd, "revert", 6) == 0) 
    {
        revert(atoi(argv[2]));
        serialize();
    } 
    else if(strncmp(cmd, "add", 3) == 0) 
    {
        add_file(argv[2]);
        serialize();
    } 
    else if(strncmp(cmd, "delete", 6) == 0) 
    {
        delete(argv[2]);
        serialize();
        return 0;
    } else 
    {
        printf(NO_CMD);
    }
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
        printf("Initialized at revision number %d.\n", revision);

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
            build_dirs(buf);
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

char push() {
    /* to server */
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
    printf("Serializing revision number %s.\n", rev);
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

/* Matching file system tree in the store folder */
void build_dirs(char* path) {
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
