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
#include <sched/elf.h>

extern void uthreadInit(struct ThreadInfo *info, void *start, uint64_t arg1, uint64_t arg2, uint64_t userspaceStackpointer);

union elfBuf {
	struct ElfHeader header;
	struct ElfPHEntry phEntry;

};

static int createProcess(struct Process **proc, struct ThreadInfo *mainThread) {
	int error;

	mainThread->priority = 1;
	mainThread->jiffiesRemaining = TIMESLICE_BASE << 1;
	mainThread->cpuAffinity = 1;

	struct Process *proc2 = kmalloc(sizeof(struct Process));
	if (!proc2) {
		error = -ENOMEM;
		goto ret;
	}
	memset(proc2, 0, sizeof(struct Process));
	mainThread->process = proc2;
	proc2->pid = 1;

	//char *newCwd = (char *)proc2 + sizeof(struct Process);
	//memcpy(newCwd, cwd, cwdLen);
	//proc2->cwd = newCwd;

	struct Inode *stdout = getInodeFromPath(rootDir, "/dev/tty0");
	error = fsOpen(stdout, &proc2->inlineFDs[1]);
	if (error) {
		goto freeProcess;
	}

	*proc = proc2;
	return 0;

	freeProcess:
	kfree(proc2);
	ret:
	return error;
}

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

int execInit(const char *fileName) {
	int error;
	uintptr_t kernelStackBottom = (uintptr_t)allocKPages(THREAD_STACK_SIZE, PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE | PAGE_FLAG_INUSE);
	if (!kernelStackBottom) {
		error = -ENOMEM;
		goto ret;
	}
	struct ThreadInfo *mainThread = (struct ThreadInfo *)(kernelStackBottom + THREAD_STACK_SIZE - sizeof(struct ThreadInfo));

	struct Process *proc;
	error = createProcess(&proc, mainThread);
	if (error) goto deallocMainThread;

	struct File f;
	struct Inode *inode = getInodeFromPath(rootDir, fileName);
	if (!inode) {
		error = -ENOENT;
		goto freeProcess;
	}
	error = fsOpen(inode, &f);
	if (error) goto freeProcess;

	struct ElfHeader header;
	ssize_t read = fsRead(&f, &header, sizeof(struct ElfHeader)); //read elf header
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

	struct ElfPHEntry phEntry;
	error = fsSeek(&f, header.phOff, SEEK_SET);
	if (error) goto closef;
	for (unsigned int i = 0; i < header.phnum; i++) {
		fsRead(&f, &phEntry, sizeof(struct ElfPHEntry));
	}

	return 0;
	
	closef:
	freeProcess:
	kfree(proc);
	deallocMainThread:
	deallocPages((void *)kernelStackBottom, THREAD_STACK_SIZE);
	ret:
	return error;
}