#include <sched/process.h>
#include <stdbool.h>
#include <mm/memset.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>
#include <arch/tlb.h>
#include <errno.h>
#include <print.h>
#include <userspace.h>

#include <arch/map.h>

static bool mmapSame(struct MemoryEntry *a, struct MemoryEntry *b) {
	return (a->flags == b->flags);
}

static int findLowerHigher(struct MemoryEntry **lower, struct MemoryEntry **higher, struct Process *proc, uintptr_t newStart) {
	struct MemoryEntry *cur = proc->firstMemEntry;
	*lower = NULL;
	*higher = NULL;
	while (cur) {
		uintptr_t curStart = (uintptr_t)cur->vaddr;
		if (newStart < curStart) {
			*higher = cur;
			break;
		}
		*lower = cur;
		cur = cur->next;
	}
	return 0;
}

static void getVaddr(struct MemoryEntry *lower, struct MemoryEntry *higher, struct Process *proc, struct MemoryEntry *entry) {
	if (entry->flags & MMAP_FLAG_FIXED) {
		entry->flags &= ~MMAP_FLAG_FIXED;
		return;
	}
	uintptr_t newEnd = (uintptr_t)entry->vaddr + entry->size;
	uintptr_t lowEnd;
	if (lower) {
		lowEnd = (uintptr_t)lower->vaddr + lower->size;
	} else {
		lowEnd = (uintptr_t)entry->vaddr;
	}

	uintptr_t highStart;
	if (higher) {
		highStart = (uintptr_t)higher->vaddr;
	} else {
		highStart = newEnd;
	}
	if (lowEnd <= (uintptr_t)entry->vaddr && highStart >= newEnd) {
		return; //Use the hint
	}
	//hint is bad
	entry->vaddr = (void *)((uintptr_t)proc->lastMemEntry->vaddr + proc->lastMemEntry->size);
}

static struct MemoryEntry *deleteEntry(struct MemoryEntry *entry) {
	if (entry->flags & MMAP_FLAG_SHARED) {
		acquireSpinlock(&entry->shared->lock);
		int refs = --(entry->shared->refCount);
		releaseSpinlock(&entry->shared->lock);
		if (!refs) {
			if (entry->flags & MMAP_FLAG_ANON) {
				for (int i = 0; i < entry->shared->nrofPages; i++) {
					deallocPhysPage(entry->shared->phys[i]);
				}
			}
			kfree(entry->shared);
		}
	}
	struct MemoryEntry *next = entry->next;
	kfree(entry);
	return next;
}

int mmapCreateEntry(void **out, struct Process *proc, struct MemoryEntry *newEntry) {
	int error = 0;
	uintptr_t newStart = (uintptr_t)newEntry->vaddr;
	struct MemoryEntry *new = kmalloc(sizeof(*new));
	if (!new) {
		error = -ENOMEM;
		goto ret;
	}
	memcpy(new, newEntry, sizeof(*new));

	acquireSpinlock(&proc->memLock);

	struct MemoryEntry *lower;
	struct MemoryEntry *higher;
	if (!new->vaddr) {
		new->vaddr = (void *)((uintptr_t)proc->lastMemEntry->vaddr + proc->lastMemEntry->size);
		newStart = (uintptr_t)new->vaddr;
	}
	if ((error = findLowerHigher(&lower, &higher, proc, newStart)) != 0) {
		goto release;
	}
	getVaddr(lower, higher, proc, new);
	if (out) {
		*out = new->vaddr;
	}
	uintptr_t newEnd = newStart + newEntry->size;

	new->prev = lower;
	if (lower) {
		uintptr_t lowStart = (uintptr_t)lower->vaddr; //guaranteed to be lower or equal
		uintptr_t lowEnd = lowStart + lower->size;
		if (newStart > lowEnd || (newStart == lowEnd && !mmapSame(lower, new))) {
			lower->next = new;
		} else if (mmapSame(lower, new)) {
			//merge
			lower->size = ((newEnd > lowEnd)? newEnd : lowEnd) - lowStart;
			kfree(new);
			new = lower;
			newStart = (uintptr_t)new->vaddr;
			newEnd = newStart + new->size;
		} else {
			lower->next = new;

			size_t llSize = newStart - lowStart;
			ssize_t lhSize = lowEnd - newEnd;
			if (lhSize > 0) {
				struct MemoryEntry *lh = kmalloc(sizeof(*lh));
				if (!lh) {
					error = -ENOMEM;
					goto release;
				}
				lh->vaddr = (void *)newEnd;
				lh->size = lhSize;

				lh->sharedOffset = lower->sharedOffset + (newEnd - lowStart);
				lh->shared = lower->shared;
				if (lh->shared) {
					acquireSpinlock(&lh->shared->lock);
					lh->shared->refCount++;
					releaseSpinlock(&lh->shared->lock);
				}

				lh->flags = lower->flags;
				lh->prev = new;
				new->next = lh;
				lh->next = higher;
				if (higher) {
					higher->prev = lh;
				} else {
					proc->lastMemEntry = lh;
				}
				new = lh;
			}
			if (!llSize) {
				struct MemoryEntry *prev = lower->prev;
				/*memcpy(lower, new, sizeof(*lower));
				lower->prev = prev;
				new = lower;*/
				deleteEntry(lower);
				new->prev = prev;
				if (prev) {
					prev->next = new;
				} else {
					proc->firstMemEntry = new;
				}
			} else {
				lower->size = llSize;
			}
			//dealloc newStart to min(newEnd, lowEnd)
			size_t deallocSize = ((newEnd > lowEnd)? lowEnd : newEnd) - newStart;
			deallocPages((void *)newStart, deallocSize);
		}
	} else {
		proc->firstMemEntry = new;
	}

	while (true) {
		new->next = higher;
		if (!higher) {
			proc->lastMemEntry = new;
			break;
		}
		higher->prev = new;
		uintptr_t highStart = (uintptr_t)higher->vaddr;
		if (highStart >= newEnd) {
			break;
		}
		//uintptr_t highEnd = highStart + higher->size;
		size_t overlapSize = newEnd - highStart;
		if (overlapSize < higher->size) {
			higher->vaddr += overlapSize;
			higher->size -= overlapSize;
			higher->sharedOffset += overlapSize;
			break;
		}
		higher = deleteEntry(higher);
	}

	release:
	releaseSpinlock(&proc->memLock);
	ret:
	return error;
}

struct MemoryEntry *mmapGetEntry(struct Process *proc, void *vaddr) {
	struct MemoryEntry *cur = proc->firstMemEntry;
	uintptr_t addr = (uintptr_t)vaddr;
	while (cur) {
		uintptr_t curStart = (uintptr_t)cur->vaddr;
		if (addr >= curStart && addr < curStart + cur->size) {
			break;
		}
		cur = cur->next;
	}
	return cur;
}

void mmapDestroy(struct Process *proc) {
	struct MemoryEntry *entry = proc->firstMemEntry;
	while (entry) {
		entry = deleteEntry(entry);
	}

	mmUnmapUserspace();
	tlbReloadCR3Local();
}

void *sysMmap(void *addr, size_t size, int flags, int fd, uint64_t offset) {
	//fd and offset are ignored for now
	int error;
	if (addr) {
		error = validateUserPointer(addr, size);
		if (error) {
			goto ret;
		}
		if ((uintptr_t)addr % PAGE_SIZE || size % PAGE_SIZE || size == 0) {
			error = -EINVAL;
			goto ret;
		}
	}

	if (!(flags & MMAP_FLAG_ANON)) {
		error = -ENOSYS; //file mmap is not supported yet
		goto ret;
	}
	if (flags & MMAP_FLAG_SHARED) {
		flags &= ~MMAP_FLAG_SHARED;
	} else {
		flags |= MMAP_FLAG_COW;
	}

	struct MemoryEntry new = {
		.vaddr = addr,
		.size = size,
		.flags = flags
	};
	void *ret;
	error = mmapCreateEntry(&ret, getCurrentThread()->process, &new);

	pageFlags_t pFlags = PAGE_FLAG_CLEAN | PAGE_FLAG_INUSE | PAGE_FLAG_USER;
	if (flags & MMAP_FLAG_WRITE) {
		pFlags |= PAGE_FLAG_WRITE;
	}
	if (flags & MMAP_FLAG_EXEC) {
		pFlags |= PAGE_FLAG_EXEC;
	}
	allocPageAt(ret, size, pFlags);

	return ret;

	ret:
	return (void *)((int64_t)error);
}

int sysMunmap(void *vaddr, size_t size) {
	int error = validateUserPointer(vaddr, size);
	if (error) {
		goto ret;
	}
	if ((uintptr_t)vaddr % PAGE_SIZE || size % PAGE_SIZE) {
		error = -EINVAL;
		goto ret;
	}

	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->memLock);

	struct MemoryEntry *cur = proc->firstMemEntry;
	uintptr_t addr = (uintptr_t)vaddr;
	uintptr_t end = addr + size;
	while (cur) {
		uintptr_t curStart = (uintptr_t)cur->vaddr;
		uintptr_t curEnd = curStart + cur->size;
		if (end <= curStart || addr >= curEnd) {
			cur = cur->next;
			continue;
		}
		ssize_t lSize = addr - curStart;
		ssize_t hSize = curEnd - end;
		struct MemoryEntry *next = cur->next;
		if (hSize > 0) {
			struct MemoryEntry *h = kmalloc(sizeof(*h));
			if (!h) {
				error = -ENOMEM;
				goto release;
			}
			h->vaddr = (void *)end;
			h->size = (size_t)hSize;
			h->shared = cur->shared;
			if (h->shared) {
				acquireSpinlock(&h->shared->lock);
				h->shared->refCount++;
				releaseSpinlock(&h->shared->lock);
			}
			h->sharedOffset = cur->sharedOffset + end - curStart;
			h->next = cur->next;
			if (!cur->next) {
				proc->lastMemEntry = h;
			}
			h->prev = cur;
			cur->next = h;

			cur->size= addr - curStart;
		}
		if (lSize > 0) {
			cur->size = lSize;
		} else {
			if (cur->prev) {
				cur->prev->next = cur->next;
				if (cur->next) {
					proc->lastMemEntry = cur->prev;
				}
			} else {
				proc->firstMemEntry = cur->next;
			}
			deleteEntry(cur);
		}
		cur = next;
	}
	deallocPages(vaddr, size);

	release:
	releaseSpinlock(&proc->memLock);
	ret:
	return error;
}