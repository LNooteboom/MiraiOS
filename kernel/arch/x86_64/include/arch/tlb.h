#ifndef INCLUDE_ARCH_TLB_H
#define INCLUDE_ARCH_TLB_H

#include <stdint.h>

/*
Initializes tlb shootdown interrupt
*/
void tlbInit(void);

/*
Invalidates a set of tlb entries on all cpus
*/
void tlbInvalidateGlobal(void *base, uint64_t nrofPages);

/*
Invalidates a set of tlb entries on this cpu only
*/
void tlbInvalidateLocal(void *base, uint64_t nrofPages);

/*
Reloads cr3 on all cpus
*/
void tlbReloadCR3(void);

#endif