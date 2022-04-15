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


int main(int argc, char *argv[]) {
    pid_t pid;
    unsigned long begin_va, end_va;
    unsigned long **fpt;
    unsigned long va;
    unsigned long pa;
    int i, j, count;
    void *pte;

    // check
    if (argc != 5 || strcmp(argv[1], "-i")) {
        printf("Usage %s -i pid begin_va end_va\n", argv[0]);
        return -1;
    }
    pid      = atoi(argv[2]);
    begin_va = atol(argv[3]);
    end_va   = atol(argv[4]);

    // align pmd
    begin_va = begin_va - (begin_va & ((1 << 21) - 1));
    end_va   = end_va - 1;
    end_va   = end_va - (end_va & ((1 << 21) - 1)) + (1 << 21);

    // allocate space
    fpt = malloc(((end_va - begin_va) >> 21) * sizeof(unsigned long *));
    pte = mmap(NULL, ((end_va - begin_va) >> 21) * (1 << 12),
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    if (!fpt || !pte) {
        printf("allocate fail.\n");
        return -1;
    }

    // expose pte
    struct expose_pte_args ep_args = {
        .pid             = pid,
        .begin_fpt_vaddr = (unsigned long)fpt,
        .end_fpt_vaddr   = (unsigned long)(fpt + ((end_va - begin_va) >> 21)),
        .begin_pte_vaddr = (unsigned long)pte,
        .end_pte_vaddr   = (unsigned long)(pte + ((end_va - begin_va) >> 21) * (1 << 12)),
        .begin_vaddr     = begin_va,
        .end_vaddr       = end_va,
    };

    if (syscall(436, &ep_args)) {
        printf("expose_pte fail.\n");
        return -1;
    }

    // dump results
    count = 0;
    for (i = 0; i < (end_va - begin_va) >> 21; ++i) {
        for (j = 0; j < 512; ++j){
            if (fpt[i] && (fpt[i][j] % 4 == 3)) {
                va = begin_va + (i << 21) + (j << 12);
                pa = (fpt[i][j] >> 12) & (((unsigned long)1 << 36) - 1);
                printf("va%d %p pa%d %p\n", count, (void *)va, count, (void *)pa);
                count++;
            }
        }
    }

    return 0;
}
