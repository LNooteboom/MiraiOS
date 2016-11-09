#include <mm/init.h>

#include <global.h>
#include <param/mmap.h>
#include <param/bootinfo.h>

void initMm(void) {
	if (bootInfo->flags & (0 << 11)) {
		pageInit((struct mmap*)(bootInfo->mmap), bootInfo->mmapSize);
	} else {
		sprint("\e[44mmmap is required but not present");
		while(true);
	}
}

