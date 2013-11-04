#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>

#define BUF_SIZE 1024
#define NO_CMD "Not a valid command. Try Nit help.\n"



/* Populate current directory with required files for starting a repo. 
 * Returns 0 (False) on failure and 1 (True) on success. */
char init_repo();
char add_file(const char* path);
char push();

GHashTable *hash;

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

char init_repo() {
    //Check to see if existing repo. Load that one.
    //Otherwise, create the new repo.
    GHashTable* hash = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(hash, "Virginia", "Richmond");
    g_hash_table_insert(hash, "Texas", "Austin");
    g_hash_table_insert(hash, "Ohio", "Columbus");
    printf("There are %d keys in the hash\n", g_hash_table_size(hash));
    printf("The capital of Texas is %s\n", g_hash_table_lookup(hash, "Texas"));
    gboolean found = g_hash_table_remove(hash, "Virginia");
    printf("The value 'Virginia' was %sfound and removed\n", found ? "" : "not ");
    g_hash_table_destroy(hash);
    

    if(hash == NULL) {
        printf("The nit structure is not initialized");
        hash = g_hash_table_new(g_str_hash, g_str_equal);
    }
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

    /* Find the hash and insert the pair into the hashtable */
    return TRUE;
}

char push() {
    return FALSE;
}
