#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>

#define BUF_SIZE 1024
#define SHA512_SIZE 128
#define NO_CMD "Not a valid command. Try Nit help.\n"



/* Populate current directory with required files for starting a repo. 
 * Returns 0 (False) on failure and 1 (True) on success. */
char init_repo();
char add_file(const char* path);
char push();
//void saveToDisk();
const char* hashfile(const char* filepath);
char copy_file(char* src_path, char* dest_path);
char commit();
void err_handler(const char* msg);

GHashTable *hash;
int revision = -1;

int main(int argc, char** argv) {
    printf("Welcome to Nit\n");
    init_repo();
    if(argc <= 1) {
        printf(NO_CMD);
        return 0;
    }
    
    printf("Init new Nit repo\n");
    if(init_repo() == FALSE) {
        printf("Nit init failed\n");
        exit(1);
    }

    char* cmd = argv[1];
    if(strcmp(cmd, "update") == 0) {
        printf("Update source code\n");
        return 0;
    }
    
    /* Commands from this point on require at least one parameter */
    if(argc <= 2) {
        printf("Command need more arguments. Try Nit help. \n");
        return 0;
    }

    if(strncmp(cmd, "add", 3) == 0) {
        printf("Add command\n");
        return 0;
    }
    if(strncmp(cmd, "delete", 6) == 0) {
        printf("Delete command\n");
        return 0;
    }

    printf(NO_CMD);
    return 0;
}

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

/* Check to see if the revision and hashtable are initialized. If not,
 * initilize them from a database file stored on the filesystem */
char init_repo() {
       if(hash == NULL || revision < 0) {
        hash = g_hash_table_new(g_str_hash, g_str_equal);
        /* Read from the text file and map each thing */
        FILE *load = fopen(".nitdb", "r");
        if(load == NULL) 
            err_handler("Failed to open data store");

        /* Allocated buffer for reading filesys */
        char *tmp, *line, key;
        line = malloc(sizeof(char) * (1 + BUF_SIZE));
        tmp = malloc(sizeof(char) * (1 + BUF_SIZE));
        if(line == NULL || tmp == NULL)
            err_handler("Memory allocation failed in initialization");
        line[BUF_SIZE] = '\0';

        /* Read active revision number */
        size_t buf_size = BUF_SIZE;
        if(getline(&line, &buf_size, load) != -1) 
            revision = atoi(line);
        else
            err_handler("Failed to read revision number!");

        /* Populate hashtable */
        key = 1;
        while(getline(&line, &buf_size, load) != -1) {
            if(key) 
                strcpy(tmp, line);
            else 
                g_hash_table_insert(hash, tmp, line);
            
            key = !key;
        }
        free(line);
    }
    system("mkdir .nit");
    return TRUE;
}    

char add_file(const char* path) {
    //Add a new entry in the hashtable, populate with the hash checksum.
    if(hash == NULL) {
        printf("initialization is wrong!");
        exit(1);
    }
    gboolean tracked = g_hash_table_contains(hash, (gconstpointer) path);
    if(tracked) {
        printf("This file is already being tracked by Nit!\n");
        return FALSE;
    }
    
    /* Maybe my functionality is confused... this updating functionality should
     * be with the commit or something */
    /* Find the hash and insert the pair into the hashtable */
    const char* sha512 = hashfile(path);
    gboolean equal = strcmp(sha512, g_hash_table_lookup(hash, path));
    if(!equal) {
        g_hash_table_insert(hash, (void*) path, (void *) sha512);
        /* Make a copy of the file */
    }
    return TRUE;
}

/* Commit : push, but for local. for each key in the hash, see if the sum is still the same. Update
 * revision */
char commit() {
    if(hash == NULL) 
        return FALSE;
    
    GList* key_list = g_hash_table_get_keys(hash);
    while(key_list) {
        const char* curr_hash = hashfile(key_list->data);
        gconstpointer old_key = (gconstpointer) key_list->data;
        const char* old_hash = g_hash_table_lookup(hash, old_key);
        if(strcmp(curr_hash, old_hash) != 0) {
            /* Do some changes */
        }
        key_list = key_list->next;
    }
    return TRUE;
}

char push() {
    /* to server */
    return FALSE;
}

void serialize() {
    if(hash == NULL)
        return; 

    FILE *f = fopen("database", "w");
    if(f == NULL) {
        printf("Error serializing database!");
        return;
    }

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, hash);
    while(g_hash_table_iter_next(&iter, &key, &value)) {
        printf("Key: %s, Value: %s.\n", key, value);
        fprintf(f, "%s\n%s\n", key, value);
    }
}
