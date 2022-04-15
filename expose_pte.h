#ifndef _EXPOSE_PTE_H
#define _EXPOSE_PTE_H

#include <unistd.h>

struct expose_pte_args {
	// PID of the target task to expose pte
	// (can be the caller task or others)
	pid_t pid;

	// begin userspace VA of the flattened page table
	unsigned long begin_fpt_vaddr;
	// end userspace VA of the flattened page table
	unsigned long end_fpt_vaddr;

	// begin userspace VA of the remapped PTE table
	unsigned long begin_pte_vaddr;
	// end userspace VA of the remapped PTE table
	unsigned long end_pte_vaddr;

	// begin of userspace VA to expose PTE mappings
	unsigned long begin_vaddr;
	// end of userspace VA to expose PTE mappings
	unsigned long end_vaddr;
};

unsigned long *expose_pte_single (pid_t pid, unsigned long vaddr);

#endif /* _EXPOSE_PTE_H */
