#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define MY_SYSCALL_NUMBER 548

int main(void)
{
    long ret;

    ret = syscall(MY_SYSCALL_NUMBER);

    printf("Retour du syscall : %ld\n", ret);

    return 0;
}
