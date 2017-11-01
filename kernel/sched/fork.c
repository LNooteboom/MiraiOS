#include <sched/process.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/pagemap.h>
#include <mm/memset.h>
#include <sched/readyqueue.h>
#include <syscall.h>
#include <modules.h>

extern void initForkRetThread(thread_t newThread, thread_t parent);

/*static int deleteMem(struct Process *proc, unsigned int limit) {

}*/

static int copyMem(struct Process *proc, struct Process *newProc) {
	int error;
	struct MemoryEntry *entries = proc->pmem.entries;
	struct MemoryEntry *newEntries = kmalloc(sizeof(struct MemoryEntry) * proc->pmem.nrofEntries);
	if (!newEntries) {
		error = -ENOMEM;
		goto ret;
	}

	unsigned int i;
	for (i = 0; i < proc->pmem.nrofEntries; i++) {
		newEntries[i].vaddr = entries[i].vaddr;
		newEntries[i].size = entries[i].size;
		newEntries[i].shared = entries[i].shared;

		if (entries[i].flags & MEM_FLAG_SHARED) {
			newEntries[i].flags = entries[i].flags;
			entries[i].shared->refCount++;
		} else {
			entries[i].flags |= MEM_FLAG_SHARED;
			newEntries[i].flags = entries[i].flags;

			uintptr_t alignedDiff = (uintptr_t)(entries[i].vaddr) % PAGE_SIZE;
			unsigned long nrofPages = (entries[i].size + alignedDiff) / PAGE_SIZE;
			if ((entries[i].size + alignedDiff) % PAGE_SIZE) {
				nrofPages++;
			}
			uintptr_t alignedAddr = (uintptr_t)(entries[i].vaddr) - ((uintptr_t)(entries[i].vaddr) % PAGE_SIZE);

			struct SharedMemory *shared = kmalloc(sizeof(struct SharedMemory) + (nrofPages - 1) * sizeof(physPage_t));
			if (!shared) {
				error = -ENOMEM;
				goto freeEveryThing;
			}

			shared->nrofPhysPages = nrofPages;
			shared->alignedVaddr = alignedAddr;
			for (unsigned long i = 0; i < nrofPages; i++) {
				shared->phys[i] = mmGetPageEntry(alignedAddr);
				alignedAddr += PAGE_SIZE;
			}

			newEntries[i].shared = entries[i].shared = shared;
			shared->refCount = 2;
			shared->vaddr = entries[i].vaddr;
			shared->size = entries[i].size;
			shared->flags = entries[i].flags;
		}
	}

	newProc->pmem.entries = newEntries;
	newProc->pmem.nrofEntries = proc->pmem.nrofEntries;

	return 0;

	freeEveryThing:
	for (unsigned int j = 0; j < i; j++) {
		entries[i].shared->refCount--;
		if (entries[i].shared->refCount <= 1) {
			kfree(entries[i].shared);
			entries[i].flags &= ~MEM_FLAG_SHARED;
		}
	}
	kfree(newEntries);
	ret:
	return error;
}

int forkRet(void) {
	thread_t curThread = getCurrentThread();
	struct Process *proc = curThread->process;
	struct MemoryEntry *entries = proc->pmem.entries;

	for (unsigned int i = 0; i < proc->pmem.nrofEntries; i++) {
		pageFlags_t flags = PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_USER | PAGE_FLAG_SHARED;
		if (entries[i].flags & MEM_FLAG_WRITE) {
			flags |= PAGE_FLAG_COW;
		}
		//if (entries[i].flags & MEM_FLAG_EXEC) {
			flags |= PAGE_FLAG_EXEC;
		//}
		struct SharedMemory *shared = entries[i].shared;
		for (unsigned int j = 0; j < shared->nrofPhysPages; j++) {
			mmMapPage(shared->alignedVaddr + j * PAGE_SIZE, shared->phys[j], flags);
		}
	}

	return 0;
}

int sysFork(void) {
	thread_t curThread = getCurrentThread();
	struct Process *curProc = curThread->process;
	int error;

	struct Process *newProc = kmalloc(sizeof(struct Process));
	memset(newProc, 0, sizeof(struct Process));

	error = copyMem(curProc, newProc);

	newProc->addressSpace = mmCreateAddressSpace();

	//create a main thread for the new process
	struct ThreadInfo *mainThread = allocKStack();
	if (!mainThread) {
		error = -ENOMEM;
		//todo
	}
	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 0;
	mainThread->process = newProc;

	newProc->mainThread = mainThread;

	initForkRetThread(mainThread, curThread);
	readyQueuePush(mainThread);

	return newProc->pid;
}

int registerForkSyscall(void) {
	registerSyscall(17, sysFork);
	return 0;
}
MODULE_INIT(registerForkSyscall);