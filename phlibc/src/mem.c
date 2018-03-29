#include <stdlib.h>
#include <sys/mman.h>
#include <uapi/syscalls.h>
#include <errno.h>
#include <stdbool.h>

#include <stdio.h>

#define HEAP_MAX		((void *)(0x80000000UL))
#define MMAP_THRESHOLD	(256*1024)
#define PAGE_SIZE		0x1000

#define HEAP_PROT		(PROT_READ | PROT_WRITE)
#define HEAP_FLAGS		(HEAP_PROT | MAP_ANON | MAP_PRIVATE)

struct HeapBlock {
	unsigned long prev;
	unsigned long next;
};
struct FreeBlock {
	struct FreeBlock *prev;
	struct FreeBlock *next;
};

static struct HeapBlock *firstBlock;
static struct HeapBlock *lastBlock;
static struct FreeBlock *firstFreeBlock;
static struct FreeBlock *lastFreeBlock; //clean is always lastFreeBlock
//static struct HeapBlock *firstCleanBlock;

void *mmap(void *addr, size_t size, int prot, int flags, int fd, off_t offset) {
	flags |= prot;
	if (!addr) {
		addr = HEAP_MAX; //allocate above HEAP_MAX
	}
	void *ret = sysMmap(addr, size, flags, fd, offset);
	int error = (long)ret;
	if (error < 0) {
		errno = error;
		return MAP_FAIL;
	}
	return ret;
}

int munmap(void *addr, size_t size) {
	int ret = sysMunmap(addr, size);
	if (ret) {
		errno = ret;
		return -1;
	}
	return 0;
}

static struct HeapBlock *nextBlock(struct HeapBlock *block) {
	return block + (block->next >> 1);
}
static struct HeapBlock *prevBlock(struct HeapBlock *block) {
	return block - (block->prev >> 1);
}
static void setNext(struct HeapBlock *block, unsigned long size, int used) {
	block->next = (size << 1) | used;
}
/*static void setPrev(struct HeapBlock *block, unsigned long size, int used) {
	block->prev = (size << 1) | used;
}*/
static int isNextUsed(struct HeapBlock *block) {
	return block->next & 1;
}
static int isPrevUsed(struct HeapBlock *block) {
	return block->prev & 1;
}
static unsigned long getNextSize(struct HeapBlock *block) {
	return block->next >> 1;
}
static unsigned long getPrevSize(struct HeapBlock *block) {
	return block->prev >> 1;
}

static struct FreeBlock *getFreeBlock(struct HeapBlock *block) {
	return (struct FreeBlock *)(block + 1);
}
static struct HeapBlock *getHeapBlock(struct FreeBlock *freeBlock) {
	return (struct HeapBlock *)(freeBlock - 1);
}

static void delFreeBlock(struct FreeBlock *fb) {
	if (fb->prev) {
		fb->prev->next = fb->next;
	} else {
		/*if (firstCleanBlock == firstFreeBlock) {
			firstCleanBlock = fb->next;
		}*/
		firstFreeBlock = fb->next;
	}
	if (fb->next) {
		fb->next->prev = fb->prev;
	} else {
		lastFreeBlock = fb->prev;
	}
}

int __PHMallocInit(void) {
	firstBlock = sysMmap(NULL, PAGE_SIZE, HEAP_FLAGS, 0, 0);
	if (firstBlock == MAP_FAIL) {
		return -1;
	}
	setNext(firstBlock, (PAGE_SIZE / sizeof(*firstBlock)) - 1, 0);
	lastBlock = nextBlock(firstBlock);
	lastBlock->prev = firstBlock->next;

	firstFreeBlock = getFreeBlock(firstBlock);
	lastFreeBlock = getFreeBlock(firstBlock);
	return 0;
}

static struct HeapBlock *findBlock(unsigned long nrofBlocks, bool clean) {
	struct FreeBlock *fb = (clean)? lastFreeBlock : firstFreeBlock;
	if (!fb) {
		return NULL;
	}
	struct HeapBlock *block = getHeapBlock(fb);

	while (true) {
		if (!isNextUsed(block) && getNextSize(block) >= nrofBlocks) {
			break;
		}
		fb = fb->next;
		if (!fb) {
			return NULL;
		}
		block = getHeapBlock(fb);
	}
	return block;
}

static void splitBlock(struct HeapBlock *start, unsigned long nrofBlocks) {
	struct FreeBlock *fb = getFreeBlock(start);
	if (nrofBlocks >= getNextSize(start) - 1) { // - 1 for freeBlock
		start->next |= 1;
		nextBlock(start)->prev = start->next;
		delFreeBlock(fb);
		return;
	}
	unsigned long oldBlocks = getNextSize(start);
	struct HeapBlock *end = nextBlock(start);
	setNext(start, nrofBlocks, 1);
	struct HeapBlock *newEnd = nextBlock(start);
	newEnd->prev = start->next;
	setNext(newEnd, oldBlocks - nrofBlocks, 0);
	end->prev = newEnd->next;

	//Move freeBlock
	struct FreeBlock *newFB = getFreeBlock(newEnd);
	newFB->next = fb->next;
	newFB->prev = fb->prev;
	if (firstFreeBlock == fb) {
		firstFreeBlock = newFB;
	} else {
		fb->prev->next = newFB;
	}
	/*if (firstCleanBlock == fb) {
		firstCleanBlock = newFB;
	}*/
	if (lastFreeBlock == fb) {
		lastFreeBlock = newFB;
	} else {
		fb->next->prev = newFB;
	}
}

static struct FreeBlock *findFreeBlock(struct FreeBlock **next, struct HeapBlock *start) {
	struct FreeBlock *fb = lastFreeBlock;
	*next = NULL;
	while (true) {
		if (!fb || (uintptr_t)fb < (uintptr_t)start) {
			return fb;
		}
		*next = fb;
		fb = fb->prev;
	}
}

static void mergeBlock(struct HeapBlock *start, struct HeapBlock *end) {
	struct FreeBlock *fb;
	if (start->prev && !isPrevUsed(start)) {
		
		struct HeapBlock *prevStart = prevBlock(start);
		fb = getFreeBlock(prevStart);
		setNext(prevStart, getPrevSize(start) + getNextSize(start), 0);
		//prevStart should now point to end
		end->prev = prevStart->next;

		start = prevStart;
	} else {
		fb = getFreeBlock(start);

		//insert free block
		struct FreeBlock *nextFree;
		struct FreeBlock *prevFree = findFreeBlock(&nextFree, start);
		fb->prev = prevFree;
		if (prevFree) {
			prevFree->next = fb;
		} else {
			firstFreeBlock = fb;
		}
		fb->next = nextFree;
		if (nextFree) {
			nextFree->prev = fb;
		} else {
			lastFreeBlock = fb;
		}
	}

	if (end->next && !isNextUsed(end)) {
		setNext(start, getNextSize(start) + getNextSize(end), 0);
		nextBlock(start)->prev = start->next;

		delFreeBlock(getFreeBlock(end));
	}
}

static void *doAlloc(size_t size, bool clean) {
	if (size >= MMAP_THRESHOLD) {
		return 1; //TODO
	}

	unsigned long nrofBlocks = (size / sizeof(struct HeapBlock)) + 1;
	if (size % sizeof(struct HeapBlock)) {
		nrofBlocks++;
	}
	struct HeapBlock *start = findBlock(nrofBlocks, clean);
	if (!start) {
		//errno = -ENOMEM;
		//return NULL; //TODO expand heap
		unsigned long more = nrofBlocks;
		if (!isPrevUsed(lastBlock)) {
			more -= getPrevSize(lastBlock);
		}

		size_t sz = more * sizeof(struct HeapBlock);
		if (sz % PAGE_SIZE) {
			sz -= sz % PAGE_SIZE;
			sz += PAGE_SIZE;
		}
		long error = (long)sysMmap(lastBlock + 1, sz, HEAP_FLAGS | MAP_FIXED, 0, 0);
		if (error < 0) {
			errno = error;
			return NULL;
		}

		setNext(lastBlock, sz / sizeof(struct HeapBlock), 0);
		struct HeapBlock *end = nextBlock(lastBlock);
		end->prev = lastBlock->next;

		struct HeapBlock *start2 = lastBlock;
		lastBlock = end;
		mergeBlock(start2, end);

		start = findBlock(nrofBlocks, clean);
		if (!start) {
			//fputs("phlibc: Heap is broke!\n", stderr);
			return NULL;
		}
	}
	splitBlock(start, nrofBlocks);
	return start + 1;
}

void free(void *mem) {
	struct HeapBlock *start = ((struct HeapBlock *)mem) - 1;
	struct HeapBlock *end = nextBlock(start);
	setNext(start, getNextSize(start), 0);
	end->prev = start->next;

	mergeBlock(start, end);
}

void *malloc(size_t size) {
	return doAlloc(size, false);
}

void *calloc(size_t nmemb, size_t size) {
	return doAlloc(nmemb * size, true);
}