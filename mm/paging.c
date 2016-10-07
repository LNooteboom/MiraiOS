#include <mm/paging.h>

#include <global.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>


uint8_t allocPage(void *addr) {
	void *page = allocPhysPage();
	if (page != NULL) {
		mapPage(addr, page);
		return 1;
	}
	return 0;
}

uint8_t deallocPage(void *addr) {
	void *page = getPhysPage(addr);
	if (page != NULL) {
		deallocPhysPage(page);
		unmapPage(addr);
		return 1;
	}
	return 0;
}
