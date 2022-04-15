#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "expose_pte.h"

int main(int argc, char *argv[]) {
    struct expose_pte_args ep_args;
    unsigned long begin_va, end_va;
    unsigned long va, pa;
    unsigned long **fpt;
    void *ptep;
    int i, j, count;

    // Check
    if (argc != 5 || strcmp(argv[1], "-i")) {
        printf("Usage %s -i pid begin_va end_va\n", argv[0]);
        exit(-1);
    }

    // Align pmd
    begin_va = atol(argv[3]);
    begin_va = begin_va - (begin_va & ((1 << 21) - 1));
    end_va   = atol(argv[4]) - 1;
    end_va   = end_va - (end_va & ((1 << 21) - 1)) + (1 << 21);

    // Allocate space
    fpt  = malloc(((end_va - begin_va) >> 21) * sizeof(unsigned long *));
    ptep = mmap(NULL, ((end_va - begin_va) >> 21) * (1 << 12),
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (!fpt || !ptep) {
        printf("allocate fail.\n");
        exit(-1);
    }

    // Expose pte
    ep_args = (struct expose_pte_args) {
        .pid             = atoi(argv[2]),
        .begin_fpt_vaddr = (unsigned long)(fpt),
        .end_fpt_vaddr   = (unsigned long)(fpt + ((end_va - begin_va) >> 21)),
        .begin_pte_vaddr = (unsigned long)(ptep),
        .end_pte_vaddr   = (unsigned long)(ptep + ((end_va - begin_va) >> 21) * (1 << 12)),
        .begin_vaddr     = begin_va,
        .end_vaddr       = end_va,
    };
    if (syscall(436, &ep_args)) {
        printf("expose_pte fail.\n");
        exit(-1);
    }

    // Dump results
    count = 0;
    for (i = 0; i < (end_va - begin_va) >> 21; ++i)
        for (j = 0; j < 512; ++j)
            if (fpt[i] && (fpt[i][j] % 4 == 3)) {
                count++;
                va = begin_va + (i << 21) + (j << 12);
                pa = (fpt[i][j] >> 12) & (((unsigned long)1 << 36) - 1);
                printf("va%d %p pa%d %p\n", count, (void *)va, count, (void *)pa);
            }

    return 0;
}
