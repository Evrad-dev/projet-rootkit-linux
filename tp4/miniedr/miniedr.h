#ifndef MINI_EDR_H
#define MINI_EDR_H

#define MAX_PATH_LEN 256
#define TASK_COMM_LEN 16

#define EVENT_EXEC      1
#define EVENT_FILE_OPEN 2

#define JAIL_BLOCK_ETC  1
#define JAIL_BLOCK_PROC 2
#define JAIL_BLOCK_ROOT 4

struct event {
    unsigned int pid;
    unsigned int ppid;
    unsigned int uid;
    unsigned int type;
    char comm[TASK_COMM_LEN];
    char path[MAX_PATH_LEN];
};

struct jail_policy {
    unsigned int flags;
};

#endif
