#ifndef INCLUDE_MM_MMAP_H
#define INCLUDE_MM_MMAP_H

#include <sched/process.h>

int mmapCreateEntry(void **out, struct Process *proc, struct MemoryEntry *newEntry);

/*
Get the memoryEntry belonging to vaddr.
memLock on process must be held.
*/
struct MemoryEntry *mmapGetEntry(struct Process *proc, void *vaddr);

void mmapDestroy(struct Process *proc);

#endif