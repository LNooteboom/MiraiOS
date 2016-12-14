#include <param/main.h>

#include <global.h>
#include <param/bootinfo.h>
#include <param/mmap.h>

void initParam(void) {
	//initialize all addresses
	bootInfo = (struct multiBootInfo*)((void*)(bootInfo) + &VMEM_OFFSET);

	bootInfo->cmdLine += &VMEM_OFFSET;
	bootInfo->modsAddr += &VMEM_OFFSET;
	bootInfo->symAddr += &VMEM_OFFSET;
	bootInfo->mmap += &VMEM_OFFSET;
	bootInfo->drivesAddr += &VMEM_OFFSET;
	bootInfo->configTable += &VMEM_OFFSET;
	bootInfo->bootloaderName += &VMEM_OFFSET;
	bootInfo->apmTable += &VMEM_OFFSET;

	//Do not increment vbe info, as that will be accessed from v86 mode
}
