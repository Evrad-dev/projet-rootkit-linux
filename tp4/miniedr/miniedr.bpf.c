#include "vmlinux.h"

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#include "miniedr.h"

char LICENSE[] SEC("license") = "GPL";

/*
 * Ring buffer pour envoyer les événements
 * du noyau vers l'espace utilisateur.
 */
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 16 * 1024 * 1024);
} events SEC(".maps");

/*
 * Politique de jail :
 * PID -> règles appliquées à ce processus.
 */
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 256);
    __type(key, __u32);
    __type(value, struct jail_policy);
} jail_map SEC(".maps");

/*
 * Hook appelé lorsqu'un processus ouvre un fichier.
 *
 * SEC("lsm.s/...") signifie que le programme est
 * "sleepable", ce qui permet notamment d'utiliser
 * bpf_d_path().
 */
SEC("lsm.s/file_open")
int BPF_PROG(file_open_hook, struct file *file, int ret)
{
    struct event *e;
    char comm[TASK_COMM_LEN];
    __u32 pid;
    struct jail_policy *policy;
    if (ret != 0)
        return ret;
    pid = bpf_get_current_pid_tgid() >> 32;

    policy = bpf_map_lookup_elem(&jail_map, &pid);

    if (policy && file && file->f_path.dentry) {
    char path[MAX_PATH_LEN];

    path[0] = '\0';

    bpf_d_path(&file->f_path, path, sizeof(path));

    if ((policy->flags & JAIL_BLOCK_ETC) &&
        path[0] == '/' &&
        path[1] == 'e' &&
        path[2] == 't' &&
        path[3] == 'c' &&
        path[4] == '/') {
        return -13;
    }

    if ((policy->flags & JAIL_BLOCK_PROC) &&
        path[0] == '/' &&
        path[1] == 'p' &&
        path[2] == 'r' &&
        path[3] == 'o' &&
        path[4] == 'c' &&
        path[5] == '/') {
        return -13;
    }

    if ((policy->flags & JAIL_BLOCK_ROOT) &&
        path[0] == '/' &&
        path[1] == 'r' &&
        path[2] == 'o' &&
        path[3] == 'o' &&
        path[4] == 't' &&
        path[5] == '/') {
        return -13;
    }
}
    e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e)
        return ret;

    e->pid = bpf_get_current_pid_tgid() >> 32;
    e->ppid = 0;
    e->uid = bpf_get_current_uid_gid() & 0xffffffff;
    e->type = EVENT_FILE_OPEN;

    bpf_get_current_comm(&comm, sizeof(comm));
    __builtin_memcpy(e->comm, comm, sizeof(e->comm));

    e->path[0] = '\0';

    if (file && file->f_path.dentry) {
        bpf_d_path(&file->f_path, e->path, sizeof(e->path));
    }

    bpf_ringbuf_submit(e, 0);

    return ret;
}

/*
 * Hook appelé lorsqu'un processus tente d'exécuter
 * un programme.
 */
SEC("lsm/bprm_check_security")
int BPF_PROG(exec_hook, struct linux_binprm *bprm)
{
    struct event *e;

    e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);
    if (!e)
        return 0;

    e->pid = bpf_get_current_pid_tgid() >> 32;
    e->ppid = 0;
    e->uid = bpf_get_current_uid_gid() & 0xffffffff;
    e->type = EVENT_EXEC;

    bpf_get_current_comm(e->comm, sizeof(e->comm));

    e->path[0] = '\0';

    if (bprm && bprm->filename) {
        bpf_core_read_str(
            e->path,
            sizeof(e->path),
            bprm->filename
        );
    }

    bpf_ringbuf_submit(e, 0);

    return 0;
}
