#include <mm/paging.h>

#include <global.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>
#include <spinlock.h>

#define KERNEL_OFFSET 0xFFFFFFFF80100000

spinlock_t kernelVMemLock;

bool allocCleanPage(uintptr_t addr, physPageFlags_t flags) {
	physPage_t page = allocCleanPhysPage();
	if (page != NULL) {
		mmMapPage(addr, page, flags);
		return true;
	}
	return false;
}

bool allocPage(uintptr_t addr, physPageFlags_t flags) {
	physPage_t page = allocPhysPage();
	if (page != NULL) {
		mmMapPage(addr, page, flags);
		return true;
	}
	return false;
}

bool deallocPage(uintptr_t addr) {
	physPage_t page = mmGetPageEntry(addr);
	if (page != NULL) {
		deallocPhysPage(page);
		mmUnmapPage(addr);
		return true;
	}
	return false;
}