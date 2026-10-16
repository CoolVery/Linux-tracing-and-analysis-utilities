#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *a = malloc(100);
    if (a) {
        strcpy(a, "hello");
        printf("   a = \"%s\"\n", a);
        free(a);          
    }

    char *b = malloc(200);
    if (b) {
        strcpy(b, "утечка");
        printf("   b = \"%s\"\n", b);
    }

    return 0;
}