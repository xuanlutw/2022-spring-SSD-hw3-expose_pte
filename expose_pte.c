
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "expose_pte.h"

unsigned long *expose_pte_single (pid_t pid, unsigned long vaddr) {
    struct expose_pte_args ep_args;
    unsigned long begin_vaddr, end_vaddr;
    unsigned long *fpt;
    void *ptep;

    // Align pmd
    begin_vaddr = vaddr - (vaddr & ((1 << 21) - 1));
    end_vaddr   = begin_vaddr + (1 << 21);

    ptep = mmap(NULL, 1 << 12, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (!ptep) {
        printf("allocate error.\n");
        exit(-1);
    }
    ep_args = (struct expose_pte_args) {
        .pid             = pid,
        .begin_fpt_vaddr = (unsigned long)(&fpt),
        .end_fpt_vaddr   = (unsigned long)(&fpt + 1),
        .begin_pte_vaddr = (unsigned long)(ptep),
        .end_pte_vaddr   = (unsigned long)(ptep + (1 << 12)),
        .begin_vaddr     = begin_vaddr,
        .end_vaddr       = end_vaddr,
    };
    if (syscall(436, &ep_args) || (fpt != ptep)) {
        printf("expose pte fail.\n");
        exit(-1);
    }

    return ptep;
}

