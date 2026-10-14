#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <bpf/bpf.h>

#include "miniedr.h"

#define JAIL_MAP_PATH "/sys/fs/bpf/miniedr/jail_map"

int main(int argc, char **argv)
{
    int map_fd;
    unsigned int pid;
    struct jail_policy policy = {0};

    if (argc < 2) {
        fprintf(stderr,
                "Usage: %s <pid> [etc] [proc] [root]\n",
                argv[0]);
        return 1;
    }

    pid = strtoul(argv[1], NULL, 10);

    /*
     * Lecture des options de jail.
     */
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "etc") == 0) {
            policy.flags |= JAIL_BLOCK_ETC;
        }
        else if (strcmp(argv[i], "proc") == 0) {
            policy.flags |= JAIL_BLOCK_PROC;
        }
        else if (strcmp(argv[i], "root") == 0) {
            policy.flags |= JAIL_BLOCK_ROOT;
        }
        else {
            fprintf(stderr,
                    "Option inconnue : %s\n",
                    argv[i]);
            return 1;
        }
    }

    /*
     * Ouverture de la jail_map épinglée.
     */
    map_fd = bpf_obj_get(JAIL_MAP_PATH);

    if (map_fd < 0) {
        fprintf(stderr,
                "Erreur : impossible d'ouvrir %s\n",
                JAIL_MAP_PATH);
        perror("bpf_obj_get");
        return 1;
    }

    /*
     * Ajout / modification de la politique
     * pour le PID indiqué.
     */
    if (bpf_map_update_elem(
            map_fd,
            &pid,
            &policy,
            BPF_ANY) < 0) {

        perror("bpf_map_update_elem");
        close(map_fd);
        return 1;
    }

    printf("Jail appliqué au PID %u\n", pid);
    printf("Flags = %u\n", policy.flags);

    if (policy.flags & JAIL_BLOCK_ETC)
        printf("  - /etc bloqué\n");

    if (policy.flags & JAIL_BLOCK_PROC)
        printf("  - /proc bloqué\n");

    if (policy.flags & JAIL_BLOCK_ROOT)
        printf("  - /root bloqué\n");

    close(map_fd);

    return 0;
}
