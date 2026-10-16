#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>



int main(void) {
    for (int i = 0; i < 100; i++) {
        printf("line %d\n", i);
    }
    for (int i = 0; i < 50; i++) {
        access("/etc/hosts", F_OK);
    }

    for (int i = 0; i < 5; i++) {
        usleep(100000); 
    }

    printf("Готово.\n");
    return 0;
}