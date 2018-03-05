#include <mm/init.h>
#include "heap.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sched/process.h>
#include <mm/memset.h>
#include <mm/memset.h>

void mmInit(void) {
	mmInitPaging();
	mmInitVirtPaging();
	initHeap();
}

static void setProcMemSize(struct MemoryEntry *entry, size_t size) {
	entry->size = size;
	if (entry->flags & MEM_FLAG_SHARED) {
		acquireSpinlock(&entry->shared->lock);
		entry->shared->size = size;
		entry->shared->nrofPhysPages = sizeToPages(size);
		//TODO phys
		releaseSpinlock(&entry->shared->lock);
	}
}

void *sysSbrk(int64_t new) {
	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->memLock);
	struct MemoryEntry *brk = proc->brkEntry;
	void *ret = (void *)((uintptr_t)(brk->vaddr) + brk->size);

	if (new < 0) {
		new = -new;
		int64_t alignedSize = brk->size - (brk->size % PAGE_SIZE);
		int64_t diff = brk->size - alignedSize;
		if (diff >= new) {
			//brk->size -= new;
			setProcMemSize(brk, brk->size - new);
			goto ret;
		}
		if ((int64_t)brk->size - new < THREAD_STACK_SIZE) {
			ret = NULL;
			goto ret;
		}
		new -= diff;
		//brk->size -= diff;
		setProcMemSize(brk, brk->size - diff);
		deallocPages((void *)((uintptr_t)(brk->vaddr) + brk->size - new), new);
	} else { //new >= 0
		int64_t alignedSize = align(brk->size, PAGE_SIZE);
		int64_t diff = alignedSize - brk->size;
		if (diff >= new) {
			//brk->size += new;
			setProcMemSize(brk, brk->size + new);
			goto ret;
		}
		new -= diff;
		//brk->size += diff;
		setProcMemSize(brk, brk->size + diff);
		allocPageAt((void *)((uintptr_t)(brk->vaddr) + brk->size), new,
			PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
		brk->size += new;
	}

	ret:
	releaseSpinlock(&proc->memLock);
	return ret;
}