#include <param/main.h>

#include <param/bootinfo.h>
#include <param/mmap.h>

#define VMEMOFFSET 0xC0000000

void initParam(void) {
	//initialize all addresses
	bootInfo = (struct multiBootInfo*)((void*)(bootInfo) + VMEMOFFSET);

	bootInfo->cmdLine += VMEMOFFSET;
	bootInfo->modsAddr += VMEMOFFSET;
	bootInfo->symAddr += VMEMOFFSET;
	//bootInfo->mmap = (struct mmap*)( (void*)(bootInfo->mmap) + VMEMOFFSET);
	bootInfo->mmap += VMEMOFFSET;
	bootInfo->drivesAddr += VMEMOFFSET;
	bootInfo->configTable += VMEMOFFSET;
	bootInfo->bootloaderName += VMEMOFFSET;
	bootInfo->apmTable += VMEMOFFSET;

	//Do not increment vbe info, as that will be accessed from v86 mode
}
