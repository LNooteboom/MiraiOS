#include <param/main.h>

#include <global.h>
#include <param/bootinfo.h>
#include <param/mmap.h>

struct mmap *paramMmap;

void paramInit(void) {
	//initialize all addresses
	bootInfo = (struct multiBootInfo*)((uintptr_t)bootInfo + (uintptr_t)&VMEM_OFFSET);

	paramMmap = (struct mmap*)((uintptr_t)bootInfo->mmap + (uintptr_t)&VMEM_OFFSET);
	//Do not increment vbe info, as that will be accessed from v86 mode
}
