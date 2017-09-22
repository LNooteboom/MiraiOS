#include <sched/process.h>
#include <sched/readyqueue.h>

#include <stdint.h>
#include <errno.h>
#include <mm/paging.h>
#include <fs/fs.h>
#include <print.h>

extern void uthreadInit(struct ThreadInfo *info, void *start, uint64_t arg1, uint64_t arg2, uint64_t userspaceStackpointer);

int createInitProcess(void) {
	int error;
	uintptr_t kernelStackBottom = (uintptr_t)allocKPages(THREAD_STACK_SIZE, PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE);
	if (!kernelStackBottom) {
		error = -ENOMEM;
		goto ret;
	}
	struct ThreadInfo *mainThread = (struct ThreadInfo *)(kernelStackBottom + THREAD_STACK_SIZE - sizeof(struct ThreadInfo));

	//kernel address space becomes init address space
	void *start = (void*)0x1000;
	allocPageAt(start, 0x1000, PAGE_FLAG_CLEAN | PAGE_FLAG_EXEC | PAGE_FLAG_USER | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE);
	//allocPageAt((void*)0x2000, 0x1000, PAGE_FLAG_USER | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE);

	struct File f;
	struct Inode *inode = getInodeFromPath(rootDir, "init");
	if (!inode) {
		error = -ENOENT;
		goto deallocMainThread;
	}
	if ((error = fsOpen(inode, &f)) < 0) {
		goto deallocMainThread;
	}
	fsRead(&f, start, 0x1000);

	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 1;

	uthreadInit(mainThread, start, 0, 0, 0x1000);

	readyQueuePush(mainThread);

	return 0;

	deallocMainThread:
	deallocPages((void *)kernelStackBottom, THREAD_STACK_SIZE);
	ret:
	return error;
}