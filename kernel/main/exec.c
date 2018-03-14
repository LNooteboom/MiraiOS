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
#include <sched/elf.h>
#include <modules.h>

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

	pageFlags_t flags = PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_USER;
	if (entry->flags & PHFLAG_EXEC) {
		flags |= PAGE_FLAG_EXEC;
	}
	if (entry->flags & PHFLAG_WRITE) {
		flags |= PAGE_FLAG_WRITE;
	}
	//readable flag is ignored
	allocPageAt(vaddr, align(entry->pMemSz, entry->alignment), flags);

	error = fsRead(f, (void *)entry->pVAddr, entry->pFileSz);
	if (error != (int)entry->pFileSz) {
		if (error >= 0) {
			error = -EINVAL;
		}
		goto deallocP;
	}

	return 0;

	deallocP:
	deallocPages(vaddr, entry->pMemSz);
	return error;
}

static int execCommon(thread_t mainThread, const char *fileName, void **startAddr, void **sp) {
	int error;
	//Open the executable
	struct File f;
	struct Inode *inode = getInodeFromPath(rootDir, fileName);
	if (!inode) {
		error = -ENOENT;
		goto ret;
	}
	error = fsOpen(inode, &f);
	if (error) goto ret;

	//Get the ELF header
	struct ElfHeader header;
	ssize_t read = fsRead(&f, &header, sizeof(struct ElfHeader));
	if (read != sizeof(struct ElfHeader)) {
		//file too small or error occured during read
		error = -EINVAL;
		if (read < 0) {
			error = read;
		}
		goto closef;
	}
	error = checkElfHeader(&header);
	if (error) goto closef;

	struct Process *proc = mainThread->process;

	//loop over all program header entries
	struct ElfPHEntry phEntry;
	for (unsigned int i = 0; i < header.phnum; i++) {
		error = fsSeek(&f, header.phOff + (i * sizeof(struct ElfPHEntry)), SEEK_SET);
		if (error) goto closef;
		ssize_t r = fsRead(&f, &phEntry, sizeof(struct ElfPHEntry));
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

				error = elfLoad(&f, &phEntry);
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

	fsClose(&f);
	return 0;

	dealloc:
	closef:
	fsClose(&f);
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

	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 1;

	mainThread->process = &initProcess;
	initProcess.mainThread = mainThread;
	initProcess.pid = 1;
	initProcess.cwd = getInodeFromPath(NULL, "/"); //Root directory must exist
	initProcess.cwd->refCount++;

	//TODO make this arch independent
	uint64_t cr3;
	asm ("mov rax, cr3" : "=a"(cr3));
	initProcess.addressSpace = cr3;

	void *start;
	void *sp;
	error = execCommon(mainThread, fileName, &start, &sp);
	if (error) goto deallocMainThread;

	uthreadInit(mainThread, start, 0, 0, sp);

	readyQueuePush(mainThread);

	return 0;

	deallocMainThread:
	deallocPages((void *)kernelStackBottom, THREAD_STACK_SIZE);
	ret:
	return error;
}

int sysExec(const char *fileName, char *const argv[] __attribute__ ((unused)), char *const envp[] __attribute__ ((unused))) {
	int error;
	int fnLen = strlen(fileName) + 1;
	if (fnLen > PAGE_SIZE) return -EINVAL;
	char namebuf[fnLen];

	error = validateUserString(fileName);
	if (error) goto ret;

	memcpy(namebuf, fileName, fnLen);

	error = fsCloseOnExec();
	if (error) goto ret;

	thread_t curThread = getCurrentThread();
	mmapDestroy(curThread->process);
	
	void *start;
	void *sp;
	error = execCommon(curThread, namebuf, &start, &sp);
	if (error) goto exitProc;

	initExecRetThread(curThread, start, 0, 0, sp);
	
	return 0;

	exitProc:
	sysExit(error);
	ret:
	return error;
}
