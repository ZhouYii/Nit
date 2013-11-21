#include "assert.h"
#include "const.c"
#include <stddef.h>
#include "stdio.h"
#include "helpers.c"

void test_hashfile();
void test_strip_newline();
void test_num_digits();
void test_itoc();
void test_skiplines();
void test_calcperm();
int main(int argc, char** argv) {
    //test_hashfile(); 
    test_strip_newline();
    test_num_digits();
    test_itoc();
    test_skiplines();
    //test_calcperm();
    return 0;
}
/*
void test_calcperm() {
    char perm[3] = {'5','3','7'};
    assert(537 == calcperm(perm));
    printf("calcperm test passed!");
}*/

void test_hashfile() {
    const char* sha512 = hashfile("test");
    assert(!strcmp(sha512, "db3974a97f2407b7cae1ae637c0030687a11913274d578492558e39c16c017de84eacdc8c62fe34ee4e12b4b1428817f09b6a2760c3f8a664ceae94d2434a593"));
    printf("hashfile test passed!\n");
}

void test_strip_newline() {
    char test[2] = {'\n', '\0'};
    strip_newline(test);
    assert('\0' == test[0]);
    printf("strip_newline test passed\n");
}

void test_num_digits() {
    assert(1 == num_digits(1));
    assert(1 == num_digits(0));
    assert(1 == num_digits(-1));
    assert(2 == num_digits(10));
    printf("num_digits test passed!\n");
}

void test_itoc() {
    char* str = itoc(57, 2);
    assert(str[0] == '5');
    assert(str[1] == '7');
    assert(str[2] == '\0');
    printf("itoc tests passed!\n");

}

void test_skiplines() {
    FILE* f = fopen(".test/skiplines", "w");
    fputs("1\n", f);
    fputs("2\n", f);
    fputs("3\n", f);
    fclose(f);
    FILE* g = fopen(".test/skiplines", "r");
    skiplines(g,1);
    char tmp;
    fgets(&tmp, 2, g);
    assert('2' == tmp);
    printf("skiplines test passed\n");
}
