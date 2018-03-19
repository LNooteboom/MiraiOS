#include <sched/process.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/pagemap.h>
#include <mm/memset.h>
#include <mm/physpaging.h>
#include <sched/readyqueue.h>
#include <userspace.h>
#include <modules.h>
#include <arch/map.h>
#include <print.h>
#include <fs/pipe.h>

extern void initForkRetThread(thread_t newThread, thread_t parent);

static uint64_t curPid = 2;


static int copyMem(struct Process *proc, struct Process *newProc) {
	int error = 0;
	struct MemoryEntry *curEntry = proc->firstMemEntry;
	struct MemoryEntry *prev = NULL;
	while (curEntry) {
		struct MemoryEntry *newEntry = kmalloc(sizeof(*newEntry));
		if (!newEntry) {
			error = -ENOMEM;
			goto ret;
		}

		memcpy(newEntry, curEntry, sizeof(*curEntry));
		newEntry->prev = prev;
		if (prev) {
			prev->next = newEntry;
		} else {
			newProc->firstMemEntry = newEntry;
		}

		prev = newEntry;

		if (curEntry->flags & MMAP_FLAG_SHARED) {
			curEntry->shared->refCount++;
			newEntry->shared = curEntry->shared;

			curEntry = curEntry->next;
			continue;
		}

		long nrofPages = sizeToPages(curEntry->size);
		curEntry->sharedOffset = 0;
		struct SharedMemory *shared = kmalloc(sizeof(*shared) + (nrofPages - 1) * sizeof(physPage_t));
		if (!shared) {
			error = -ENOMEM;
			goto ret;
		}
		memset(shared, 0, sizeof(*shared));
		shared->refCount = 2;
		shared->nrofPages = nrofPages;

		uintptr_t curAddr = (uintptr_t)curEntry->vaddr;
		for (long i = 0; i < nrofPages; i++) {
			//shared->phys[i] = mmGetPageEntry(curAddr);
			pte_t *pte = mmGetEntry(curAddr, 0);
			*pte |= PAGE_FLAG_COW | PAGE_FLAG_SHARED; //also set COW for parent process
			*pte &= ~PAGE_FLAG_WRITE;
			shared->phys[i] = *pte & PAGE_MASK;
			curAddr += PAGE_SIZE;
		}

		curEntry->shared = shared;
		curEntry->flags |= MMAP_FLAG_SHARED;
		newEntry->shared = shared;
		newEntry->flags |= MMAP_FLAG_SHARED;

		curEntry = curEntry->next;
	}

	ret:
	return error;
}

static int copyFiles(struct Process *proc, struct Process *newProc) {
	newProc->cwd = proc->cwd;
	proc->cwd->refCount++;

	struct File *shared;
	for (int i = 0; i < NROF_INLINE_FDS; i++) {
		if (proc->inlineFDs[i].flags & PROCFILE_FLAG_SHARED) {
			shared = kmalloc(sizeof(*shared));
			if (!shared) return -ENOMEM;
		} else {
			shared = NULL;
		}

		copyFD(&proc->inlineFDs[i], &newProc->inlineFDs[i], shared);
	}
	if (!proc->nrofFDs) {
		return 0;
	}

	newProc->fds = kmalloc(proc->nrofFDs * sizeof(struct ProcessFile));
	if (!newProc->fds) return -ENOMEM;
	newProc->nrofFDs = proc->nrofFDs;
	newProc->nrofUsedFDs = proc->nrofUsedFDs;

	for (int i = 0; i < newProc->nrofFDs; i++) {
		if (proc->fds[i].flags & PROCFILE_FLAG_SHARED) {
			shared = kmalloc(sizeof(*shared));
			if (!shared) return -ENOMEM;
		} else {
			shared = NULL;
		}

		copyFD(&proc->fds[i], &newProc->fds[i], shared);
	}
	return 0;
}

void linkChild(struct Process *parent, struct Process *child) {
	child->ppid = parent->pid;
	child->parent = parent;
	//insert at front
	if (parent->children) {
		child->nextChild = parent->children;
		parent->children->prevChild = child;
	}
	parent->children = child;
}

/*
Return from fork (child process)
*/
int forkRet(void) {
	thread_t curThread = getCurrentThread();
	struct Process *proc = curThread->process;
	struct MemoryEntry *entry = proc->firstMemEntry;
	while (entry) {
		pageFlags_t flags = PAGE_FLAG_CLEAN | PAGE_FLAG_USER | PAGE_FLAG_SHARED;
		if (entry->flags & MMAP_FLAG_WRITE && entry->flags & MMAP_FLAG_COW) {
			flags |= PAGE_FLAG_COW;
		} else if (entry->flags & MMAP_FLAG_WRITE) {
			flags |= PAGE_FLAG_WRITE;
		}
		if (entry->flags & MMAP_FLAG_EXEC) {
			flags |= PAGE_FLAG_EXEC;
		}
		struct SharedMemory *shared = entry->shared;
		for (unsigned int i = 0; i < shared->nrofPages; i++) {
			mmMapPage((uintptr_t)entry->vaddr + i * PAGE_SIZE, shared->phys[i], flags);
		}

		entry = entry->next;
	}
	return 0;
}

/*
Create a copy of the current process.
Will return child PID to the parent, 0 to the child
*/
int sysFork(void) {
	thread_t curThread = getCurrentThread();
	struct Process *curProc = curThread->process;
	int error;

	struct Process *newProc = kmalloc(sizeof(struct Process));
	if (!newProc) {
		error = -ENOMEM;
		goto ret;
	}
	memset(newProc, 0, sizeof(struct Process));

	acquireSpinlock(&curProc->lock);

	newProc->pid = curPid++;
	linkChild(curProc, newProc);

	acquireSpinlock(&curProc->fdLock);
	error = copyFiles(curProc, newProc);
	releaseSpinlock(&curProc->fdLock);
	if (error) goto releaseProcLock;

	error = copyMem(curProc, newProc);
	if (error) goto releaseProcLock;

	releaseSpinlock(&curProc->lock);

	newProc->addressSpace = mmCreateAddressSpace();
	if (!newProc->addressSpace) {
		error = -ENOMEM;
		goto exitProc;
	}

	//create a main thread for the new process
	struct ThreadInfo *mainThread = allocKStack();
	if (!mainThread) {
		error = -ENOMEM;
		goto exitProc;
	}
	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 0;
	mainThread->process = newProc;
	mainThread->state = THREADSTATE_SCHEDWAIT;

	newProc->mainThread = mainThread;

	acquireSpinlock(&curThread->lock);
	initForkRetThread(mainThread, curThread);
	releaseSpinlock(&curThread->lock);
	
	readyQueuePush(mainThread);

	return newProc->pid;

	releaseProcLock:
	releaseSpinlock(&curProc->lock);
	exitProc:
	exitProcess(newProc, error);
	ret:
	printk("sysFork failed! error: %d\n", error);
	return error;
}
