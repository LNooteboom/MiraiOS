#include <mm/init.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <param/mmap.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>
#include <mm/heap.h>
#include <mm/lowmem.h>
#include <print.h>

#include "map.h"

//#define LOWMEM_END 0x00100000
#define ENTRYTYPE_FREE 1

#define NROF_PAGE_STACKS 4

static bool checkMmap(struct mmap *mmap, size_t mmapSize) {
	struct mmap *currentEntry = mmap;
	uintptr_t oldEntryEnd = 0;
	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		if (currentEntry->base < oldEntryEnd) { //if unordered or overlapped
			return false;
		}
		oldEntryEnd = currentEntry->base + currentEntry->length;
		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(uint32_t));
	}
	return true;
}

void mmInitPaging(struct mmap *mmap, size_t mmapSize) {
	if (!checkMmap(mmap, mmapSize)) {
		sprint("Mmap is corrupted");
		while (true) {
			asm("hlt");
		}
	}

	uintptr_t physBssEnd = (uintptr_t)&BSS_END_ADDR - (uintptr_t)&VMEM_OFFSET;
	if (physBssEnd & (PAGE_SIZE - 1)) {
		physBssEnd &= ~(PAGE_SIZE - 1);
		physBssEnd += PAGE_SIZE;
	}
	//uintptr_t physKernelStart = (uintptr_t)&KERNEL_START_ADDR - (uintptr_t)&VMEM_OFFSET;
	struct mmap *currentEntry = mmap;
	bool firstPage = true;

	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		if (currentEntry->type == ENTRYTYPE_FREE) {
			
			uint64_t base = currentEntry->base;
			uint64_t size = currentEntry->length;

			//align base and size
			if (base & (PAGE_SIZE - 1)) {
				base &= ~(PAGE_SIZE - 1);
				base += PAGE_SIZE;
				size &= ~(PAGE_SIZE - 1);
			}
			//skip ivt + bda (for some reason mmap ignores this)
			if (base == 0) {
				base = PAGE_SIZE;
				size -= PAGE_SIZE;
			}
			sprint("Found free memory: ");
			hexprint64(base);
			sprint(" - ");
			hexprintln64(base + size);

			//allocate room in virtual address space for page stacks
			if (firstPage && size >= (NROF_PAGE_STACKS + 1) * PAGE_SIZE) {
				uintptr_t start = (uintptr_t)&BSS_END_ADDR;
				if (start & (LARGEPAGE_SIZE - 1)) { //align it with large pages
					start &= ~(LARGEPAGE_SIZE - 1);
					start += LARGEPAGE_SIZE;
				}
				uintptr_t freeBufferLarge = start;
				start += LARGEPAGE_SIZE;
				uintptr_t freeBufferSmall = start;
				start += PAGE_SIZE;

				//set new page table
				mmSetPageEntry(start, 1, base | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
				base += PAGE_SIZE;
				size -= PAGE_SIZE;

				mmInitPhysPaging(start, freeBufferSmall, freeBufferLarge);
				firstPage = false;
			}
			//dealloc lowmem (<16MiB)
			if (base < LOWMEM_SIZE) {
				size_t lowSize;
				if (base + size > LOWMEM_SIZE) {
					lowSize = LOWMEM_SIZE - base;
					size -= lowSize;
				} else {
					lowSize = size;
					size = 0;
				}
				deallocLowPhysPages(base, lowSize / PAGE_SIZE);
				base += lowSize;
				//size -= lowSize;
			}
			//dealloc highmem
			if (size != 0 && base + size > physBssEnd) {
				//uintptr_t currentPage = base;
				if (base < physBssEnd) {
					size -= physBssEnd - base;
					base = physBssEnd;
				}
				for (size_t i = 0; i < size; i += PAGE_SIZE) {
					//hexprintln64(base);
					//asm("xchg bx, bx");
					if ( !(base & (LARGEPAGE_SIZE - 1)) && size >= PAGE_SIZE) {
						//There is a better way to do this, but right now this is the simplest solution
						deallocLargePhysPage(base);
						base += LARGEPAGE_SIZE;
						i += LARGEPAGE_SIZE - PAGE_SIZE;
					} else {
						deallocPhysPage(base);
						base += PAGE_SIZE;
					}
				}
			}
		}
		//get next entry
		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(uint32_t));
	}
}
