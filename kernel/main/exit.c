#include <sched/process.h>
#include <mm/pagemap.h>
#include <mm/heap.h>
#include <mm/physpaging.h>
#include <fs/fs.h>
#include <arch/tlb.h>
#include <syscall.h>
#include <modules.h>
#include <sched/readyqueue.h>

static void unlinkChild(struct Process *proc) {
	struct Process *parent = proc->parent;
	acquireSpinlock(&parent->lock);
	if (parent->children == proc) {
		parent->children = proc->nextChild;
	} else { //not first, prevChild should be valid
		proc->prevChild->nextChild = proc->nextChild;
	}
	releaseSpinlock(&parent->lock);
}

static bool checkFilter(pid_t filter, struct Process *child) {
	if (!filter) {
		return true;
	}
	if (filter > 0 && child->pid == filter) {
		return true;
	}
	//TODO check for group id
	return false;
}

int destroyProcessMem(struct Process *proc) { //also called by exec
	//Destroy page mapping
	mmUnmapUserspace(); //POINTERS TO USERSPACE ARE NO LONGER VALID AFTER THIS CALL
	tlbReloadCR3Local();

	//dealloc process pmem entries
	for (unsigned int i = 0; i < proc->pmem.nrofEntries; i++) {
		if (proc->pmem.entries[i].flags & MEM_FLAG_SHARED) {
			acquireSpinlock(&proc->pmem.entries[i].shared->lock);
			int refcount = proc->pmem.entries[i].shared->refCount--;
			releaseSpinlock(&proc->pmem.entries[i].shared->lock);
			if (!refcount) {
				kfree(proc->pmem.entries[i].shared);
			}
			proc->pmem.entries[i].shared->refCount--;
		}
	}
	kfree(proc->pmem.entries);

	return 0;
}

void removeProcess(struct Process *proc) { //remove zombie
	//delete root page table
	deallocPhysPage(proc->addressSpace);

	//delete main thread
	//deallocThread(proc->mainThread); already done on kthreadExit

	unlinkChild(proc);
	kfree(proc);
}

void exitProcess(struct Process *proc, int exitValue) { //can also be called on failed fork
	proc->state = PROCSTATE_FINISHED;
	proc->exitValue = exitValue;

	//Orphan any children
	acquireSpinlock(&initProcess.lock);
	struct Process *child = proc->children;
	while (child) {
		acquireSpinlock(&child->lock);
		linkChild(&initProcess, child);
		struct Process *nextChild = child->nextChild;
		releaseSpinlock(&child->lock);
		child = nextChild;
	}
	releaseSpinlock(&initProcess.lock);

	//destroy memory
	destroyProcessMem(proc);
	
	//destroy files
	for (int i = 0; i < NROF_INLINE_FDS + proc->nrofFDs; i++) {
		sysClose(i);
	}

	//notify parent TODO send SIGCHLD
	struct Process *parent = proc->parent;
	acquireSpinlock(&parent->lock);
	//TODO add support for multiple threads
	thread_t parentThread = parent->mainThread;
	acquireSpinlock(&parentThread->lock);
	if (parentThread->state == THREADSTATE_PIDWAIT && checkFilter(parentThread->waitPid, proc)) {
		readyQueuePush(parentThread);
	}
	releaseSpinlock(&parentThread->lock);
	releaseSpinlock(&parent->lock);
}

void sysExit(int exitValue) {
	thread_t curThread = getCurrentThread();
	//kill all other threads
	//set current thread as mainThread

	exitProcess(curThread->process, exitValue);

	//exit thread
	curThread->detached = true;
	kthreadExit(NULL);
}

pid_t sysWaitPid(pid_t filter, int *waitStatus, int options) {
	//check if any of the current children that match the filter are status=PROCSTATE_FINISHED 
	//if so return
	pid_t ret = 0;
	bool exist = false;
	thread_t curThread = getCurrentThread();
	struct Process *proc = curThread->process;

	acquireSpinlock(&curThread->lock); //lock curThread early to prevent child exit during execution of this function
	acquireSpinlock(&proc->lock);
	struct Process *child = proc->children;
	while (child) {
		acquireSpinlock(&child->lock);
		if (checkFilter(filter, child)) {
			exist = true;
			if (child->state == PROCSTATE_FINISHED) {
				releaseSpinlock(&child->lock);
				ret = child->pid;
				break;
			}
		}
		struct Process *next = child->nextChild;
		releaseSpinlock(&child->lock);
		child = next;
	}
	releaseSpinlock(&proc->lock);
	if (ret > 0) {
		if (waitStatus) {
			*waitStatus = child->exitValue; //TODO expand this when signals get added
		}
		removeProcess(child);
		return ret;
	}
	if (!exist) {
		return -ECHILD; //child does not exist
	}
	
	//save filter
	//Wait for children to exit
	
	curThread->state = THREADSTATE_PIDWAIT;
	curThread->waitPid = filter;
	kthreadStop();

	return curThread->waitPid;
}
