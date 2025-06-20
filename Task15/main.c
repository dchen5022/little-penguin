#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_EUDYPTULA 463

int main() {
    int high_id = 0x682c;
    int low_id = 0x83e55b77;
    long res;

    /* Valid call to sys_eudyptula */
    res = syscall(SYS_EUDYPTULA, high_id, low_id);
    if (res)
        goto err;

    /* Invalid call to sys_eudyptula */
    res = syscall(SYS_EUDYPTULA, low_id, high_id);
    if (res)
        goto err;

err:
    fprintf(stderr, "sys_eudyptula failed with err=%ld\n", res);
    return 1;
}
