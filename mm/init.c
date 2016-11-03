#include <mm/init.h>

#include <global.h>
#include <param/mmap.h>
#include <param/bootinfo.h>

void initMm(void) {
	pageInit((struct mmap*)(bootInfo->mmap), bootInfo->mmapSize);
}

