#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#include <bpf/libbpf.h>

#include "miniedr.skel.h"
#include "miniedr.h"

static volatile sig_atomic_t exiting = 0;

static void sig_handler(int sig)
{
    exiting = 1;
}

/*
 * Fonction appelée automatiquement lorsqu'un
 * événement arrive dans le ring buffer.
 */
static int handle_event(void *ctx, void *data, size_t data_sz)
{
    const struct event *e = data;

    if (e->type == EVENT_EXEC) {
        printf("[EXEC] PID=%u UID=%u COMM=%s PATH=%s\n",
               e->pid,
               e->uid,
               e->comm,
               e->path);
    }
    else if (e->type == EVENT_FILE_OPEN) {
        printf("[OPEN] PID=%u UID=%u COMM=%s PATH=%s\n",
               e->pid,
               e->uid,
               e->comm,
               e->path);
    }

    fflush(stdout);

    return 0;
}

int main(void)
{
    struct miniedr_bpf *skel;
    struct ring_buffer *rb = NULL;
    int err;

    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Ouverture du skeleton BPF */
    skel = miniedr_bpf__open();

    if (!skel) {
        fprintf(stderr,
                "Erreur : impossible d'ouvrir le skeleton BPF\n");
        return 1;
    }

    /* Chargement du programme BPF */
    err = miniedr_bpf__load(skel);

    if (err) {
        fprintf(stderr,
                "Erreur : impossible de charger le programme BPF : %d\n",
                err);

        miniedr_bpf__destroy(skel);
        return 1;
    }

    /* Attachement des hooks LSM */
    err = miniedr_bpf__attach(skel);

    if (err) {
        fprintf(stderr,
                "Erreur : impossible d'attacher le programme BPF : %d\n",
                err);

        miniedr_bpf__destroy(skel);
        return 1;
    }

        /*
     * Création du répertoire de pinning.
     */
    if (mkdir("/sys/fs/bpf/miniedr", 0755) < 0 && errno != EEXIST) {
        perror("mkdir /sys/fs/bpf/miniedr");
        miniedr_bpf__destroy(skel);
        return 1;
    }

    /*
     * Épinglage de jail_map dans le système de fichiers BPF.
     */
    err = bpf_map__pin(
        skel->maps.jail_map,
        "/sys/fs/bpf/miniedr/jail_map"
    );

    if (err) {
        fprintf(stderr,
                "Erreur : impossible d'épingler jail_map : %d\n",
                err);

        miniedr_bpf__destroy(skel);
        return 1;
    }

    printf("jail_map épinglée dans /sys/fs/bpf/miniedr/jail_map\n");

    /*
     * Création du lecteur de ring buffer.
     */
    rb = ring_buffer__new(
        bpf_map__fd(skel->maps.events),
        handle_event,
        NULL,
        NULL
    );

    if (!rb) {
        fprintf(stderr,
                "Erreur : impossible de créer le ring buffer\n");

        miniedr_bpf__destroy(skel);
        return 1;
    }

    printf("miniEDR Phase 2 démarré. PID=%d\n", getpid());
    printf("Journalisation des ouvertures de fichiers et des exécutions.\n");
    printf("CTRL+C pour arrêter.\n\n");

    /*
     * On attend les événements.
     */
    while (!exiting) {
        err = ring_buffer__poll(rb, 100);

        if (err < 0) {
            fprintf(stderr,
                    "Erreur ring buffer : %d\n",
                    err);
            break;
        }
    }

    printf("\nArrêt du miniEDR.\n");

    ring_buffer__free(rb);
    miniedr_bpf__destroy(skel);

    return 0;
}
