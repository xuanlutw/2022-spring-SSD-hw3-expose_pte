#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include <unistd.h>
#include <sys/mman.h>

struct expose_pte_args {
    pid_t pid;

    unsigned long begin_fpt_vaddr;
    unsigned long end_fpt_vaddr;

    unsigned long begin_pte_vaddr;
    unsigned long end_pte_vaddr;

    unsigned long begin_vaddr;
    unsigned long end_vaddr;
};


int main(void) {
    // MAP_SHARED is IMPORTANT!!!
    void* pte = mmap(NULL, 1 << 12,
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    char* ptr1 = mmap(NULL, 2 << 12,
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    char* ptr2 = ptr1 + (1 << 12);
    unsigned long *fpt[10];

    if (!pte || !ptr1) {
        printf("mmap fail.\n");
        return -1;
    }

    strcpy(ptr1, "QQAAQQ");
    strcpy(ptr2, "rhythm");

    struct expose_pte_args args = {
        .pid             = getpid(),
        .begin_fpt_vaddr = (unsigned long)fpt,
        .end_fpt_vaddr   = (unsigned long)(fpt + 10),
        .begin_pte_vaddr = (unsigned long)pte,
        .end_pte_vaddr   = (unsigned long)(pte + (1 << 12)),
        .begin_vaddr     = (unsigned long)ptr1,
        .end_vaddr       = (unsigned long)(ptr2 + (1 << 12)),
    };

    printf("Before modify pte.\n");
    printf("ptr1: %p, %s\n", ptr1, ptr1);
    printf("ptr2: %p, %s\n", ptr2, ptr2);

    if (syscall(436, &args)) {
        printf("expose_pte fail.\n");
        return -1;
    }

    int idx = ((unsigned long)ptr1 >> 12) % 512;
    unsigned long temp = fpt[0][idx];
    fpt[0][idx] = fpt[0][idx + 1];
    fpt[0][idx + 1] = temp;

    printf("After modify pte.\n");
    printf("ptr1: %p, %s\n", ptr1, ptr1);
    printf("ptr2: %p, %s\n", ptr2, ptr2);

    return 0;
}
