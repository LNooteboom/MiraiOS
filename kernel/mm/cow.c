#include <sched/process.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <mm/memset.h>
#include <mm/mmap.h>
#include <mm/heap.h>
#include <arch/tlb.h>
#include <arch/map.h>
#include <errno.h>
#include <userspace.h>
#include <arch/cpu.h>

int mmDoCOW(uintptr_t addr) {
	int error = validateUserPointer((void *)addr, 1);
	if (error) {
		return error;
	}
	addr = alignLow(addr, PAGE_SIZE);
	if (!mmGetPageEntry(addr)) {
		return -EINVAL;
	}
	pte_t *pte = mmGetEntry(addr, 0);
	if (!(*pte & PAGE_FLAG_COW)) {
		return -EINVAL;
	}
	
	excStackPush();

	pageFlags_t oldPageFlags = (*pte & PAGE_FLAG_EXEC);

	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->memLock);
	struct MemoryEntry *memEntry = mmapGetEntry(proc, (void *)addr);

	unsigned int flags = memEntry->flags;
	flags &= ~MMAP_FLAG_SHARED;
	flags |= MMAP_FLAG_ANON | MMAP_FLAG_FIXED;

	bool skip = (flags & MMAP_FLAG_SHARED) && (memEntry->shared->refCount == 1);
	if (skip) {
		*pte = (*pte & PAGE_MASK) | oldPageFlags | PAGE_FLAG_USER | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE | PAGE_FLAG_PRESENT;
		memEntry->flags = flags;
		kfree(memEntry->shared);

		releaseSpinlock(&proc->memLock);
		error = 0;
		goto ret;
	}
	releaseSpinlock(&proc->memLock);

	physPage_t newPage = allocPhysPage();
	if (!newPage) {
		error = -ENOMEM;
		goto ret;
	}
	void *new = ioremap(newPage, PAGE_SIZE);
	if (!new) {
		error = -ENOMEM;
		goto deallocPhys;
	}

	//copy
	memcpy(new, (void *)addr, PAGE_SIZE);

	iounmap(new, PAGE_SIZE);

	
	struct MemoryEntry entry = {
		.vaddr = (void *)addr,
		.size = PAGE_SIZE,
		.flags = flags
	};
	mmapCreateEntry(NULL, proc, &entry);

	*pte = newPage | oldPageFlags | PAGE_FLAG_USER | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE | PAGE_FLAG_PRESENT;
	tlbInvalidateLocal((void *)addr, 1); //TODO ajust this for userspace multithreading
	
	error = 0;
	goto ret;
	
	deallocPhys:
	deallocPhysPage(newPage);
	ret:
	excStackPop();
	return error;
}