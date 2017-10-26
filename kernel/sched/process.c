#include <sched/process.h>
#include <sched/readyqueue.h>

#include <stdint.h>
#include <errno.h>
#include <mm/paging.h>
#include <fs/fs.h>
#include <print.h>
#include <syscall.h>
#include <mm/memset.h>
#include <mm/heap.h>
#include <mm/pagemap.h>
#include <sched/elf.h>

extern void uthreadInit(struct ThreadInfo *info, void *start, uint64_t arg1, uint64_t arg2, uint64_t userspaceStackpointer);

union elfBuf {
	struct ElfHeader header;
	struct ElfPHEntry phEntry;

};

static int checkElfHeader(struct ElfHeader *header) {
	if (!memcmp(&header->magic[0], "\x7f""ELF", 4)) {
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

	void *vaddr = (void *)entry->pVAddr;
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
	allocPageAt(vaddr, entry->pMemSz, flags);

	error = fsRead(f, vaddr, entry->pFileSz);
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

static int execCommon(thread_t mainThread, const char *fileName) {
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

	//loop over all program header entries
	struct ElfPHEntry phEntry;
	for (unsigned int i = 0; i < header.phnum; i++) {
		error = fsSeek(&f, header.phOff + (i * sizeof(struct ElfPHEntry)), SEEK_SET);
		if (error) goto closef;
		fsRead(&f, &phEntry, sizeof(struct ElfPHEntry));
		switch (phEntry.type) {
			case PHTYPE_NULL:
				break;
			case PHTYPE_LOAD:
				error = elfLoad(&f, &phEntry);
				printk("LOAD complete!\n");
				break;
			default:
				error = -EINVAL;
		}
		if (error) goto closef;
	}

	//userspace regs are always at the top of the kernel stack
	uthreadInit(mainThread, (void *)header.entryAddr, 0, 0, 0);

	return 0;
	
	closef:
	ret:
	return error;
}

int execInit(const char *fileName) {
	int error;
	uintptr_t kernelStackBottom = (uintptr_t)allocKPages(THREAD_STACK_SIZE, PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE);
	if (!kernelStackBottom) {
		error = -ENOMEM;
		goto ret;
	}
	//ThreadInfo struct is at the top of the kernel stack
	struct ThreadInfo *mainThread = (struct ThreadInfo *)(kernelStackBottom + THREAD_STACK_SIZE - sizeof(struct ThreadInfo));

	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 1;

	struct Process *proc = kmalloc(sizeof(struct Process));
	if (!proc) {
		error = -ENOMEM;
		goto ret;
	}
	memset(proc, 0, sizeof(struct Process));
	mainThread->process = proc;
	proc->pid = 1;
	proc->cwd = "/";

	struct Inode *stdout = getInodeFromPath(rootDir, "/dev/tty0");
	error = fsOpen(stdout, &proc->inlineFDs[1]);
	if (error) {
		goto freeProcess;
	}
	if (error) goto deallocMainThread;

	error = execCommon(mainThread, fileName);
	if (error) goto freeProcess;

	readyQueuePush(mainThread);

	return 0;

	freeProcess:
	kfree(proc);
	deallocMainThread:
	deallocPages((void *)kernelStackBottom, THREAD_STACK_SIZE);
	ret:
	return error;
}

int exec(const char *fileName, char *const argv[], char *const envp[]) {
	int error;
	error = validateUserString(fileName);
	if (error) goto ret;

	thread_t curThread = getCurrentThread();

	mmUnmapUserspace();

	error = execCommon(curThread, fileName);
	if (error) goto ret;

	return 0;

	ret:
	return error;
}