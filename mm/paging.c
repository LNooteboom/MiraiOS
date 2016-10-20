#include <mm/paging.h>

#include <global.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>


uint8_t allocPage(virtPage_t addr) {
	physPage_t page = allocPhysPage();
	if (page != NULL) {
		mapPage(addr, page);
		return 1;
	}
	return 0;
}

uint8_t deallocPage(virtPage_t addr) {
	physPage_t page = getPhysPage(addr);
	if (page != NULL) {
		deallocPhysPage(page);
		unmapPage(addr);
		return 1;
	}
	return 0;
}
