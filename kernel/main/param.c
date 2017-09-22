#include <param/main.h>

#include <stdint.h>
#include <param/bootinfo.h>
#include <param/mmap.h>

struct Mmap *paramMmap;

void paramInit(void) {
	//initialize all addresses
	bootInfo = (struct MultiBootInfo*)((uintptr_t)bootInfo + (uintptr_t)&VMEM_OFFSET);

	paramMmap = (struct Mmap*)((uintptr_t)bootInfo->mmap + (uintptr_t)&VMEM_OFFSET);
	//Do not increment vbe info, as that will be accessed from v86 mode
}
