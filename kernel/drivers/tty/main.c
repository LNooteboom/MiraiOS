#include "tty.h"
#include <errno.h>
#include <arch/bootinfo.h>
#include <fs/fs.h>
#include <fs/devfile.h>
#include <print.h>
#include <modules.h>
#include <mm/memset.h>
#include <mm/heap.h>

#include <arch/map.h>

static char ttyName[] = "tty0";

struct Vtty ttys[NROF_VTTYS];
struct Vtty *kernelTty = &ttys[0];
struct Vtty *currentTty = &ttys[0];

static int ttyWrite(struct File *file, const void *buffer, size_t bufSize);
static struct DevFileOps ttyOps = {
	.write = ttyWrite,
	.read = ttyRead
};

static int kernelPuts(const char *text) {
	struct Vtty *tty = kernelTty;
	int error = ttyPuts(tty, text, strlen(text));
	return error;
}

static int ttyWrite(struct File *file, const void *buffer, size_t bufSize) {
	return ttyPuts(file->inode->cachedData, buffer, bufSize);
}

int fbInitDevFiles(void) {
	ttyInitInput();

	printk("[TTY] Creating device files...\n");
	struct Inode *devDir = getInodeFromPath(rootDir, "dev");
	for (unsigned int i = 0; i < NROF_VTTYS; i++) {
		ttyName[sizeof(ttyName) - 2] = '0' + i;
		fsCreateCharDev(devDir, ttyName, &ttyOps, &ttys[i]);
	}
	return 0;
}
MODULE_INIT(fbInitDevFiles);

int fbInit(void) {
	//check if bootfb exists
	if (!bootInfo.fbAddr) {
		return -EIO;
	}
	char *vmem = ioremap(bootInfo.fbAddr, bootInfo.fbSize);
	if (!vmem) {
		return -ENOMEM;
	}
	for (size_t i = 0; i < bootInfo.fbSize; i += PAGE_SIZE) {
		*mmGetEntry((uintptr_t)vmem + i, 0) |= (1 << 3);
	}
	struct Framebuffer *bootFB = kmalloc(sizeof(struct Framebuffer));
	if (!bootFB) {
		iounmap(vmem, bootInfo.fbSize);
		return -ENOMEM;
	}

	bootFB->vmem = vmem;
	bootFB->pitch = bootInfo.fbPitch;
	bootFB->xResolution = bootInfo.fbXRes;
	bootFB->yResolution = bootInfo.fbYres;
	bootFB->isRGB = bootInfo.fbIsRgb;
	bootFB->bpp = bootInfo.fbBpp / 8;

	for (unsigned int i = 0; i < NROF_VTTYS; i++) {
		ttys[i].fb = bootFB;
		ttys[i].charWidth = bootFB->xResolution / 8;
		ttys[i].charHeight = bootFB->yResolution / FONT_HEIGHT;

		ttys[i].curFGCol = 7;

		semInit(&ttys[i].updateSem, 0);
		semInit(&ttys[i].inputAvail, 0);

		ttys[i].buf = allocKPages(SCROLLBACK_LINES * ttys[i].charWidth * sizeof(struct VttyChar),
			PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
	}
	kernelTty->focus = true;

	for (int i = 0; i < kernelTty->charWidth * SCROLLBACK_LINES; i++) {
		kernelTty->buf[i].c = 0;
	}

	setKernelStdout(kernelPuts);
	
	return 0;
}
MODULE_INIT_LEVEL(fbInit, 0);