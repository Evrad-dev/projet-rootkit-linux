#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define MY_SYSCALL_NUMBER 549

int main(void)
{
    long ret;
    long uptime_value = 0;

    ret = syscall(MY_SYSCALL_NUMBER, &uptime_value);

    printf("Retour du syscall : %ld secondes\n", ret);
    printf("Valeur copiée : %ld secondes\n", uptime_value);

    return 0;
}
