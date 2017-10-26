#include <mm/paging.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mm/init.h> //for BSS_END_ADDR
#include <mm/pagemap.h>
#include <sched/spinlock.h>
#include <mm/physpaging.h>
#include <arch/tlb.h>
#include <arch/map.h>

#define KERNEL_VMEM_END 0xFFFFFFFFC0000000

#define MIN_PHYSPAGES	16

#define PACKED_ADDR_MASK	0x0000FFFFFFFFFFFF
#define PACKED_SIZE_SHIFT	48

unsigned long totalPhysPages = 0;
atomic_ulong freePhysPages = 0;

static spinlock_t KVMemLock;

static pte_t *freeStart;

static void *KVMemEnd;

static inline void unpackAddrAndSize(pte_t **next, unsigned int *size, pte_t packed) {
	if (packed & PACKED_ADDR_MASK) {
		*next = (pte_t*)(packed | ~PACKED_ADDR_MASK);
	} else {
		*next = NULL;
	}
	*size = packed >> PACKED_SIZE_SHIFT;
}

static inline void updateAddr(pte_t *pte, pte_t *next) {
	uintptr_t inext = (uintptr_t)next;
	inext &= PACKED_ADDR_MASK;
	*pte &= PACKED_ADDR_MASK;
	*pte |= inext;
}

static inline void updateSize(pte_t *pte, unsigned int size) {
	*pte &= ~PACKED_ADDR_MASK;
	*pte |= (pte_t)size << PACKED_SIZE_SHIFT;
}

static inline pte_t packAddrAndSize(pte_t *next, unsigned int size) {
	pte_t ret = (pte_t)next;
	ret &= PACKED_ADDR_MASK;
	ret |= (pte_t)size << PACKED_SIZE_SHIFT;
	return ret;
}

static pte_t *allocVMem(uint32_t nrofPages) {
	pte_t *curFree = freeStart;
	pte_t *prev = NULL;
	pte_t *found = NULL;
	
	unsigned int curNrofPages;
	pte_t *next;
	while (true) {
		unpackAddrAndSize(&next, &curNrofPages, *curFree);
		if (curNrofPages >= nrofPages) {
			curNrofPages -= nrofPages;
			found = (pte_t *)curFree;
			//found += curNrofPages;

			if (!curNrofPages) {
				//remove entry from list
				if (!prev) {
					freeStart = next;
				} else {
					updateAddr(prev, next);
				}
			} else {
				//updateSize(curFree, nrofPages);
				pte_t *newFree = &curFree[nrofPages];
				*newFree = packAddrAndSize(next, curNrofPages);
				if (prev) {
					updateAddr(prev, newFree);
				} else {
					freeStart = newFree;
				}
			}

			break;
		}

		if (next) {
			prev = curFree;
			curFree = next;
		} else {
			break;
		}
	}

	if (!found) {
		pte_t *newEntry = mmGetEntry((uintptr_t)KVMemEnd, 0); //cannot be dereferenced until a new PT is created
		unsigned int morePagesNeeded = nrofPages;
		if (curFree && newEntry == &curFree[curNrofPages]) {
			morePagesNeeded -= curNrofPages;
		}
		unsigned int nrofPTs = morePagesNeeded / 512;
		if (morePagesNeeded % 512) nrofPTs++;

		for (unsigned int i = 0; i < nrofPTs; i++) {
			physPage_t newPT = allocCleanPhysPage();
			if (!newPT) {
				return NULL;
			}
			mmSetPageEntry((uintptr_t)KVMemEnd, 1, newPT | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
			KVMemEnd = (void *)((uintptr_t)KVMemEnd + LARGEPAGE_SIZE);
		}
		if (curFree && newEntry == &curFree[curNrofPages]) {
			newEntry = &curFree[nrofPages];
			unsigned int diff = ((uintptr_t)KVMemEnd - (uintptr_t)newEntry) / PAGE_SIZE;
			*newEntry = packAddrAndSize(NULL, diff);
			if (prev) {
				updateAddr(prev, newEntry);
			} else {
				freeStart = newEntry;
			}
			found = curFree;
		} else {
			//newEntry stays the same
			unsigned int nrofLeftOverPages = (nrofPTs * 512) - nrofPages;
			pte_t *freeEntry = &newEntry[nrofPages];
			*freeEntry = packAddrAndSize(NULL, nrofLeftOverPages);
			if (prev) {
				updateAddr(prev, freeEntry);
			} else {
				freeStart = freeEntry;
			}
			found = newEntry;
		}
	}

	return found;
}

static void deallocVMem(pte_t *start, unsigned int nrofPages) {

	pte_t *nextFree = freeStart;
	pte_t *prev = NULL;

	unsigned int nextNrofPages;
	pte_t *next;
	while (nextFree) {
		unpackAddrAndSize(&next, &nextNrofPages, *nextFree);
		if (&start[nrofPages] == nextFree) {
			//before
			//create new list entry on start
			*start = packAddrAndSize(next, nrofPages + nextNrofPages);
			if (!prev) {
				freeStart = start;
			} else {
				updateAddr(prev, start);
			}
			break;
		} else if (start == &nextFree[nextNrofPages]) {
			//after
			updateSize(nextFree, nrofPages + nextNrofPages);
			break;
		} else if ((uintptr_t)start > (uintptr_t)(&nextFree[nextNrofPages])) {
			//create new entry
			*start = packAddrAndSize(next, nrofPages);
			updateAddr(nextFree, start);
			break;
		}

		prev = nextFree;
		nextFree = next;
	}
}

void mmInitVirtPaging(void) {
	uintptr_t bssEnd = (uintptr_t)(&BSS_END_ADDR);
	if (bssEnd % LARGEPAGE_SIZE) {
		bssEnd -= bssEnd % LARGEPAGE_SIZE;
		bssEnd += LARGEPAGE_SIZE;
	}
	bssEnd += 5 * PAGE_SIZE;

	KVMemEnd = (void *)bssEnd;
	if (bssEnd % LARGEPAGE_SIZE) {
		KVMemEnd = (void *)((uintptr_t)KVMemEnd - bssEnd % LARGEPAGE_SIZE + LARGEPAGE_SIZE);
	}

	//create first vmem list entry
	pte_t *entry = mmGetEntry(bssEnd, 0);
	if (entry == KVMemEnd) {
		//create new PT
		physPage_t page = allocCleanPhysPage();
		mmSetPageEntry(bssEnd, 1, page | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
		*entry = packAddrAndSize(NULL, 512);
	} else {
		unsigned int nrofPages = ((uintptr_t)KVMemEnd - bssEnd) / PAGE_SIZE;
		*entry = packAddrAndSize(NULL, nrofPages);
	}
	freeStart = entry;
}

void *allocKPages(size_t size, pageFlags_t flags) {
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}

	if (nrofPages > freePhysPages || freePhysPages - nrofPages < MIN_PHYSPAGES) {
		return NULL;
	}
	freePhysPages -= nrofPages;

	acquireSpinlock(&KVMemLock);
	pte_t *mem = allocVMem(nrofPages);
	if (!mem) {
		releaseSpinlock(&KVMemLock);
		return NULL;
	}
	for (unsigned int i = 0; i < nrofPages; i++) {
		mem[i] = flags;
	}
	releaseSpinlock(&KVMemLock);

	return (void *)getAddrFromPte(mem, 0);
}

void allocPageAt(void *addr, size_t size, pageFlags_t flags) {
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}

	if (flags & PAGE_FLAG_INUSE) {
		freePhysPages -= nrofPages;
	}

	//flags |= PAGE_FLAG_ALLOCED;
	for (unsigned int i = 0; i < nrofPages; i++) {
		mmSetPageFlags((uintptr_t)addr + (i * PAGE_SIZE), flags);
	}
}

void deallocPages(void *addr, size_t size) {
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}

	freePhysPages += nrofPages;

	pte_t *mem = mmGetEntry((uintptr_t)addr, 0);
	for (unsigned int i = 0; i < nrofPages; i++) {
		if ((mem[i] & PAGE_FLAG_PRESENT) && (mem[i] & PAGE_FLAG_INUSE) && (mem[i] & PAGE_MASK)) {
			physPage_t page = mem[i] & PAGE_MASK;
			deallocPhysPage(page);
		}
		mem[i] = 0;
	}

	if ((uintptr_t)addr > (uintptr_t)&VMEM_OFFSET) {
		//deallocate kernel virtual memory
		deallocVMem(mem, nrofPages);
	}

	//TODO: Check if TLB inval is actually needed
	tlbInvalidateGlobal(addr, nrofPages);
}


void *ioremap(uintptr_t paddr, size_t size) {
	unsigned int paddrDiff = paddr % PAGE_SIZE;
	size += paddrDiff;
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}

	acquireSpinlock(&KVMemLock);
	pte_t *mem = allocVMem(nrofPages);
	if (!mem) {
		releaseSpinlock(&KVMemLock);
		return NULL;
	}
	paddr -= paddrDiff;
	for (unsigned int i = 0; i < nrofPages; i++) {
		mem[i] = paddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
		paddr += PAGE_SIZE;
	}
	releaseSpinlock(&KVMemLock);

	return (void *)(getAddrFromPte(mem, 0) + paddrDiff);
}

void iounmap(void *addr, size_t size) {
	unsigned int addrDiff = (uintptr_t)addr % PAGE_SIZE;
	size += addrDiff;
	addr = (void*)((uintptr_t)addr - addrDiff);
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}
	pte_t *mem = mmGetEntry((uintptr_t)addr, 0);
	for (unsigned int i = 0; i < nrofPages; i++) {
		mem[i] = 0;
	}
	//ioremapped memory is always in kernel vmem
	deallocVMem(mem, nrofPages);

	tlbInvalidateGlobal(addr, nrofPages);
}
