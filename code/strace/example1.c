#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("/dsds", O_RDONLY);
    if (fd >= 0) close(fd);

    fd = openat(AT_FDCWD, "/etc/os-release", O_RDONLY);
    if (fd >= 0) close(fd);

    FILE *fp = fopen("/etc/hosts", "r");
    if (fp) fclose(fp);

    return 0;
}