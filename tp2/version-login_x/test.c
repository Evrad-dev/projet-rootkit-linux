#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#define VERSION_MODIFIED _IO('v', 1)
#define VERSION_RESET    _IO('v', 2)

int main(void)
{
    int fd;
    char buffer[256];
    ssize_t n;
    int modified;

    fd = open("/dev/version", O_RDWR);

    if (fd < 0) {
        perror("open /dev/version");
        return 1;
    }

    n = read(fd, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    buffer[n] = '\0';

    printf("Version initiale : %s\n", buffer);

    modified = ioctl(fd, VERSION_MODIFIED);

    if (modified < 0)
        perror("VERSION_MODIFIED");
    else
        printf("Version modifiée ? %s\n",
               modified ? "oui" : "non");

    const char *new_version = "Version modifiee par TP2\n";

    if (write(fd, new_version, strlen(new_version)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    modified = ioctl(fd, VERSION_MODIFIED);

    if (modified < 0)
        perror("VERSION_MODIFIED");
    else
        printf("Après modification : %s\n",
               modified ? "oui" : "non");

    if (ioctl(fd, VERSION_RESET) < 0) {
        perror("VERSION_RESET");
        close(fd);
        return 1;
    }

    modified = ioctl(fd, VERSION_MODIFIED);

    if (modified < 0)
        perror("VERSION_MODIFIED");
    else
        printf("Après reset : %s\n",
               modified ? "oui" : "non");

    close(fd);

    return 0;
}

