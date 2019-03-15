#include <sched/process.h>
#include <sched/readyqueue.h>

#include <stdint.h>
#include <errno.h>
#include <mm/paging.h>
#include <fs/fs.h>
#include <print.h>
#include <userspace.h>
#include <mm/memset.h>
#include <mm/heap.h>
#include <mm/mmap.h>
#include <mm/pagemap.h>
#include <sched/elf.h>
#include <modules.h>
#include <sched/pgrp.h>
#include <uapi/syscalls.h>
#include <arch/tlb.h>

struct Process initProcess;

extern void uthreadInit(thread_t thread, void *start, uint64_t arg1, uint64_t arg2, void *userspaceStackpointer);
extern void initExecRetThread(thread_t thread, void *start, uint64_t arg1, uint64_t arg2, void *userspaceStackpointer);


static int checkElfHeader(struct ElfHeader *header) {
	if (memcmp(&header->magic[0], "\x7f""ELF", 4)) {
		return -EINVAL;
	}
	if (header->type != 2) {
		return -EINVAL;
	}
	if (header->machine != 0x3E) {
		return -EINVAL;
	}
	if (header->phentSize != sizeof(struct ElfPHEntry)) {
		return -EINVAL;
	}
	return 0;
}

static int elfLoad(struct File *f, struct ElfPHEntry *entry) {
	int error = fsSeek(f, entry->pOffset, SEEK_SET);
	if (error) return error;

	void *vaddr = (void *)alignLow(entry->pVAddr, entry->alignment);
	error = validateUserPointer(vaddr, entry->pMemSz);
	if (error) return error;

	pageFlags_t flags = PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_USER | PAGE_FLAG_WRITE;
	if (entry->flags & PHFLAG_EXEC) {
		flags |= PAGE_FLAG_EXEC;
	}
	//readable flag is ignored
	size_t sz = align(entry->pMemSz, entry->alignment);
	allocPageAt(vaddr, sz, flags);

	error = fsRead(f, (void *)entry->pVAddr, entry->pFileSz);
	if (error != (int)entry->pFileSz) {
		if (error >= 0) {
			error = -EINVAL;
		}
		goto deallocP;
	}

	if (!(entry->flags & PAGE_FLAG_WRITE)) {
		//disable write access
		mmSetWritable(vaddr, sz, false);
		tlbInvalidateLocal(vaddr, sz / PAGE_SIZE);
	}

	return 0;

	deallocP:
	deallocPages(vaddr, entry->pMemSz);
	return error;
}

static int createStack(void **sp, uintptr_t stack, char *const argv[], char *const envp[]) {
	//copy envp strings
	int error = 0;
	int nrofEnvs = 0;
	int envLen = 0;
	long nrofArgs = 0;
	int argLen = 0;
	bool sel = false; //false = envp, true = argv

	char *const *strList = envp;
	for (int i = 0; ; i++) {
		error = validateUserPointer(strList + i, sizeof(char *));
		if (error) goto ret;

		if (!strList[i]) {
			if (!sel) {
				strList = argv;
				i = -1;
				sel = true;
				continue;
			} else {
				break;
			}
		}

		error = validateUserStringL(strList[i]);
		if (error < 0) goto ret;
		//error = string len
		error++; //include null terminator
		if (!sel) {
			envLen += error;
			nrofEnvs++;
		} else {
			argLen += error;
			nrofArgs++;
		}
	}
	uintptr_t uTop = stack + PAGE_SIZE - argLen - envLen;
	error = 0;
	//Add one for NULL-ptr termination
	nrofArgs++;
	nrofEnvs++;

	char **newArgv = (char **)(alignLow(uTop, 16) - nrofArgs * sizeof(char *) - nrofEnvs * sizeof(char *));
	//copy args
	for (int i = 0; i < nrofArgs; i++) {
		if (!argv[i]) {
			newArgv[i] = NULL;
			break;
		}
		int len = strlen(argv[i]) + 1;
		memcpy((void *)uTop, argv[i], len);
		newArgv[i] = (char *)uTop - stack;
		uTop += len;
	}
	for (int i = 0; i < nrofEnvs; i++) {
		if (!envp[i]) {
			newArgv[i + nrofArgs] = NULL;
			break;
		}
		int len = strlen(envp[i]) + 1;
		memcpy((void *)uTop, envp[i], len);
		newArgv[i + nrofArgs] = (char *)uTop - stack;
		uTop += len;
	}
	newArgv[-1] = (char *)(nrofArgs - 1); //argc
	*sp = &newArgv[-1];

	ret:
	return error;
}

static int execCommon(thread_t mainThread, struct File *executable, void **startAddr, void **sp) {
	int error;

	//Get the ELF header
	struct ElfHeader header;
	ssize_t read = fsRead(executable, &header, sizeof(struct ElfHeader));
	if (read != sizeof(struct ElfHeader)) {
		//file too small or error occured during read
		error = -EINVAL;
		if (read < 0) {
			error = read;
		}
		goto ret;
	}
	error = checkElfHeader(&header);
	if (error) goto ret;

	struct Process *proc = mainThread->process;

	//loop over all program header entries
	struct ElfPHEntry phEntry;
	for (unsigned int i = 0; i < header.phnum; i++) {
		error = fsSeek(executable, header.phOff + (i * sizeof(struct ElfPHEntry)), SEEK_SET);
		if (error) goto ret;
		ssize_t r = fsRead(executable, &phEntry, sizeof(struct ElfPHEntry));
		if (r < 0) {
			error = r;
			goto dealloc;
		}

		switch (phEntry.type) {
			case PHTYPE_NULL:
				break;
			case PHTYPE_LOAD:
				if (phEntry.alignment % PAGE_SIZE) {
					error = -EINVAL;
					goto dealloc;
				}

				error = elfLoad(executable, &phEntry);
				if (error) goto dealloc;
				struct MemoryEntry entry = {
					.vaddr = (void *)(alignLow(phEntry.pVAddr, phEntry.alignment)),
					.size = align(phEntry.pMemSz, phEntry.alignment),
					.flags = (phEntry.flags & ~MMAP_FLAG_SHARED) | MMAP_FLAG_FIXED | MMAP_FLAG_ANON | MMAP_FLAG_COW
				};
				error = mmapCreateEntry(NULL, proc, &entry);
				if (error) goto dealloc;
				break;

			default:
				error = -EINVAL;
		}
	}
	*startAddr = (void *)header.entryAddr;

	//allocate stack
	struct MemoryEntry stackEntry = {
		.vaddr = NULL,
		.size = THREAD_STACK_SIZE,
		.flags = MMAP_FLAG_WRITE | MMAP_FLAG_ANON | MMAP_FLAG_COW
	};
	error = mmapCreateEntry(sp, proc, &stackEntry);
	if (error) goto dealloc;

	allocPageAt(*sp, THREAD_STACK_SIZE,
		PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
	*sp = (void *)((uintptr_t)*sp + THREAD_STACK_SIZE);

	return 0;

	dealloc:
	ret:
	return error;
}

int execInit(const char *fileName) {
	int error;
	uintptr_t kernelStackBottom = (uintptr_t)allocKPages(THREAD_STACK_SIZE, PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
	if (!kernelStackBottom) {
		error = -ENOMEM;
		goto ret;
	}
	//ThreadInfo struct is at the top of the kernel stack
	struct ThreadInfo *mainThread = (struct ThreadInfo *)(kernelStackBottom + THREAD_STACK_SIZE - sizeof(struct ThreadInfo));
	memset(mainThread, 0, sizeof(struct ThreadInfo));

	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 0;
	mainThread->queueEntry = &mainThread->defaultQueueEntry;
	mainThread->defaultQueueEntry.thread = mainThread;

	mainThread->process = &initProcess;
	initProcess.mainThread = mainThread;
	initProcess.pid = 1;
	initProcess.cwd = getInodeFromPath(NULL, "/"); //Root directory must exist
	initProcess.cwd->refCount++;

	procHTAdd(&initProcess);
	setpgid(&initProcess, 0);

	struct Inode *inode = getInodeFromPath(initProcess.cwd, fileName);
	if (!inode) {
		error = -ENOENT;
		goto ret;
	}

	struct File executable = {
		.inode = inode,
		.flags = SYSOPEN_FLAG_READ
	};
	error = fsOpen(&executable);
	if (error) goto ret;

	void *start;
	long *sp;
	error = execCommon(mainThread, &executable, &start, (void**)&sp);
	if (error) goto deallocMainThread;

	//create empty argv
	*(--sp) = 0; //envp
	*(--sp) = 0; //argv
	*(--sp) = 0; //argc

	uthreadInit(mainThread, start, 0, 0, sp);

	readyQueuePush(mainThread->queueEntry);

	goto closeExec;

	deallocMainThread:
	deallocPages((void *)kernelStackBottom, THREAD_STACK_SIZE);
	closeExec:
	fsClose(&executable);
	ret:
	return error;
}

int sysExec(const char *fileName, char *const argv[], char *const envp[]) {
	if (!fileName || !argv || !envp) {
		return -EINVAL;
	}
	int error = validateUserString(fileName);
	if (error) {
		return error;
	}

	//Open the executable
	struct Inode *cwd = getCurrentThread()->process->cwd;
	struct Inode *inode = getInodeFromPath(cwd, fileName);
	if (!inode) {
		error = -ENOENT;
		goto ret;
	}

	if (!fsAccessAllowed(inode, PERM_X)) {
		error = -EPERM;
		goto ret;
	}

	struct File executable = {
		.inode = inode,
		.flags = SYSOPEN_FLAG_READ
	};
	error = fsOpen(&executable);
	if (error) return error;;

	error = fsCloseOnExec();
	if (error) goto closeExec;

	uintptr_t stack = (uintptr_t)allocKPages(PAGE_SIZE, PAGE_FLAG_WRITE);
	if (!stack) {
		error = -ENOMEM;
		goto closeExec;
	}
	void *sp;
	error = createStack(&sp, stack, argv, envp);
	if (error) goto deallocStack;
	uintptr_t spDiff = PAGE_SIZE - ((uintptr_t)sp - stack);

	thread_t curThread = getCurrentThread();
	mmapDestroy(curThread->process);
	
	void *start;
	void *userSP;
	error = execCommon(curThread, &executable, &start, &userSP);
	if (error) goto deallocStack;

	fsClose(&executable);

	userSP = (void *)((uintptr_t)userSP - spDiff);
	memcpy(userSP,sp, spDiff);

	char **list = (char **)userSP + 1;
	bool sel = false;
	for (int i = 0; ; i++) {
		if (!list[i]) {
			if (!sel) {
				sel = true;
				continue;
			} else {
				break;
			}
		}
		list[i] += alignLow((uintptr_t)userSP, PAGE_SIZE);
	}

	deallocPages((void *)stack, PAGE_SIZE);

	initExecRetThread(curThread, start, 0, 0, userSP);
	
	return 0;

	deallocStack:
	fsClose(&executable);
	deallocPages((void *)stack, PAGE_SIZE);
	printk("err: %d", error);
	struct Process *proc = getCurrentThread()->process;
	proc->exitInfo.si_signo = SIGABRT;
	signalExit();
	closeExec:
	fsClose(&executable);
	ret:
	return error;
}
