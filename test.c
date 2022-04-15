#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "expose_pte.h"

int main() {
    char *pg1, *pg2;
    unsigned long *ptep;
    unsigned long temp;
    int idx;

    pg1 = mmap(NULL, 2 << 12, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pg2 = pg1 + (1 << 12);
    if (!pg1) {
        printf("allocate error.\n");
        exit(-1);
    }
    strcpy(pg1, "QQAAQQ");
    strcpy(pg2, "rhythm");

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

    printf("After modify pte.\n");
    printf("pg1 = %p, %s\n", pg1, pg1);
    printf("pg2 = %p, %s\n", pg2, pg2);

    return 0;
}
