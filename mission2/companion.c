
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/companion_demo"
#define TEST_PATH "./device_test.txt"
#define BUFFER_SIZE 256

static void show_help(void)
{
    printf("=== Linux Companion Program ===\n");
    printf("Available commands:\n");
    printf("  help    - Display available commands\n");
    printf("  status  - Display program status\n");
    printf("  check [path] - Read a device or test file\n");
}

static int check_device(const char *path)
{
    char buffer[BUFFER_SIZE];
    int fd;
    ssize_t bytes_read;

    fd = open(path, O_RDONLY);

    if (fd == -1) { 
        fprintf(stderr, "Cannot open %s: %s\n",
        	path, strerror(errno));
        return 1;
    }

    printf("Device opened successfully.\n");

    bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1) {
        fprintf(stderr, "Cannot read device: %s\n",
                strerror(errno));
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Device response: %s\n", buffer);

    if (close(fd) == -1) {
        perror("Cannot close device");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3 ||
        (argc == 3 && strcmp(argv[1], "check") != 0)) {
        fprintf(stderr, "Error: expected one command.\n");
        show_help();
        return 1;
    }

    if (strcmp(argv[1], "help") == 0) {
        show_help();
        return 0;
    }

    if (strcmp(argv[1], "status") == 0) {
        printf("Companion program is running.\n");
        printf("Device interface: %s\n", DEVICE_PATH);
        return 0;
    }

    if (strcmp(argv[1], "check") == 0) {
	if (argc > 3) {
        	fprintf(stderr, "Error: too many arguments.\n");
        	return 1;
    	}

    	const char *path = (argc == 3)
        	? argv[2]
        	: DEVICE_PATH;

    	return check_device(path);
    }

    fprintf(stderr, "Error: unknown command '%s'.\n", argv[1]);
    return 1;
}

