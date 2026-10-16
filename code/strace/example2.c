#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("PID: %d\n", getpid());
    printf("Запусти в другом терминале:\n");
    printf("  strace -p %d -tt -T\n\n", getpid());
    printf("Введи что-нибудь и нажми Enter...\n");
    fflush(stdout);

    char buf[256];
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Get: %s", buf);
    }

    printf("Готово.\n");
    return 0;
}