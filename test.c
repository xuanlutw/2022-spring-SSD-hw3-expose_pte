#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "expose_pte.h"

int main(int argc, char* argv[]) {
    char *pg1, *pg2;
    unsigned long *ptep;
    unsigned long temp;
    int idx;

    // Check arguments
    if (argc != 2) {
        printf("Usage %s flush_type\n", argv[0]);
        exit(-1);
    }

    pg1 = mmap(NULL, 2 << 12, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pg2 = pg1 + (1 << 12);
    if (!pg1) {
        printf("allocate error.\n");
        exit(-1);
    }
    strcpy(pg1, "AAAAAA");
    strcpy(pg2, "BBBBBB");

    printf("Before modify pte.\n");
    printf("pg1 = %p, %s\n", pg1, pg1);
    printf("pg2 = %p, %s\n", pg2, pg2);

    // Expose pte
    ptep = expose_pte_single(getpid(), (unsigned long)pg1);

    // Modify pte
    idx = ((unsigned long)pg1 >> 12) % 512;
    temp          = ptep[idx];
    ptep[idx]     = ptep[idx + 1];
    ptep[idx + 1] = temp;

    // Flush tlb
    switch (atoi(argv[1])) {
        case 0:
            break;
        case 1:
            syscall(437); // sys_flush
            break;
        case 2:
            syscall(438); // sys_nop
            break;
    }

    printf("After modify pte.\n");
    printf("pg1 = %p, %s\n", pg1, pg1);
    printf("pg2 = %p, %s\n", pg2, pg2);

    return 0;
}
