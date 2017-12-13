#include <arch/bootinfo.h>
#include <modules.h>
#include <io.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/memset.h>
#include <sched/spinlock.h>
#include <sched/thread.h>
#include <sched/lock.h>
#include <fs/fs.h>
#include <fs/devfile.h>
#include <print.h>

#include <arch/map.h>

#define FONT_HEIGHT 16
#define SCROLLBACK_LINES 256
#define NROF_VTTYS	8

#define FGCOL		0xD00000
#define BGCOL		0

#define BPP	4

struct FbColorInfo {
	uint8_t rSize;
	uint8_t rShift;
	uint8_t gSize;
	uint8_t gShift;
	uint8_t bSize;
	uint8_t bShift;
};

struct Framebuffer {
	char *vmem;
	unsigned int pitch;
	unsigned int xResolution;
	unsigned int yResolution;
	uint32_t bpp;

	bool isRGB;
	struct FbColorInfo colorInfo;
};

struct Vtty {
	struct Framebuffer *fb;
	spinlock_t earlyLock;

	uint16_t charWidth; //width of the screen in characters
	uint16_t charHeight; //height of the screen in characters
	uint16_t cursorX;
	uint16_t cursorY;
	char *scrollback;

	uint16_t scrollbackY;
	bool full;
	volatile bool focus;

	semaphore_t sem;
};

static int ttyWrite(struct File *file, void *buffer, size_t bufSize);

extern char font8x16[];

static struct Vtty ttys[NROF_VTTYS];
static struct Vtty *kernelTty = &ttys[0];
static struct Vtty *currentTty = &ttys[0];

static bool ttyEarly = true;

//static uint64_t lastUpdateTime = 0;
static char ttyName[] = "tty0";
static struct DevFileOps ttyOps = {
	.write = ttyWrite
};

static void drawChar(char *vmem, unsigned int newline, char c) {
	if (c < 32 || c > 126) {
		c = ' ';
	}
	unsigned int charIndex = (c - 32) * FONT_HEIGHT;
	for (unsigned int charY = 0; charY < FONT_HEIGHT; charY++) {
		unsigned char line = font8x16[charIndex++];
		for (unsigned int charX = 0; charX < 8; charX++) {
			uint32_t color = BGCOL;
			if (line & 1) {
				color = FGCOL;
				//color = 0xAA0000;
			}
			write32(vmem, color);
			vmem += BPP;
			line >>= 1;
		}
		vmem += newline;
	}
}
static int fbUpdate(struct Vtty *tty) {
	if (!tty->focus) {
		return 0;
	}
	struct Framebuffer *fb =  tty->fb;
	char *yvmem = fb->vmem;
	unsigned int newline = fb->pitch - (8 * BPP);

	unsigned int maxY = (tty->scrollbackY + tty->charHeight) % SCROLLBACK_LINES;
	for (unsigned int y = tty->scrollbackY; y != maxY; y = (y + 1) % SCROLLBACK_LINES) {
		for (unsigned int x = 0; x < tty->charWidth; x++) {
			char *addr = yvmem + (x * 8 * BPP);
			drawChar(addr, newline, tty->scrollback[y * tty->charWidth + x]);
		}
		yvmem += fb->pitch * FONT_HEIGHT;
	}
	return 0;
}

static void newline(struct Vtty *tty) {
	tty->cursorX = 0;
	tty->cursorY++;
	if (tty->cursorY >= SCROLLBACK_LINES) {
		tty->full = true;
		tty->cursorY = 0;
	}
	//clear line
	for (int x = 0; x < tty->charWidth; x++) {
		tty->scrollback[tty->cursorY * tty->charWidth + x] = 0;
	}

	//adjust scrollback pos
	int y = tty->cursorY - tty->charHeight;
	if (y < 0) {
		if (tty->full) {
			y = SCROLLBACK_LINES + y;
		} else {
			y = 0;
		}
	} else if (y >= SCROLLBACK_LINES) {
		y = y % SCROLLBACK_LINES;
	}
	tty->scrollbackY = y;
}

static int ttyPuts(struct Vtty *tty, const char *text, size_t textLen) {
	for (unsigned int i = 0; i < textLen; i++) {
		switch (text[i]) {
			case '\r':
				tty->cursorX = 0;
				break;
			case '\n':
				newline(tty);
				break;
			default:
				tty->scrollback[tty->cursorY * tty->charWidth + tty->cursorX] = text[i];
				tty->cursorX++;
				if (tty->cursorX >= tty->charWidth) {
					newline(tty);
				}
				break;
		}
	}
	if (ttyEarly) {
		fbUpdate(tty);
	} else {
		//tty->dirty = true;
		semSignal(&tty->sem);
	}
	return 0;
}

static int kernelPuts(const char *text) {
	struct Vtty *tty = kernelTty;
	acquireSpinlock(&tty->earlyLock);

	int error = ttyPuts(tty, text, strlen(text));

	releaseSpinlock(&tty->earlyLock);
	return error;
}

static int ttyWrite(struct File *file, void *buffer, size_t bufSize) {
	return ttyPuts(file->inode->cachedData, buffer, bufSize);
}

void ttyScroll(int amount) {
	acquireSpinlock(&kernelTty->earlyLock);
	int y = kernelTty->scrollbackY + amount;
	if (y < 0) {
		if (kernelTty->full) {
			y = SCROLLBACK_LINES + y;
		} else {
			y = 0;
		}
	} else if (y >= SCROLLBACK_LINES) {
		y = y % SCROLLBACK_LINES;
	}
	if (y == (kernelTty->cursorY - kernelTty->charHeight + 1) % SCROLLBACK_LINES) {
		y = (kernelTty->cursorY - kernelTty->charHeight) % SCROLLBACK_LINES;
	}
	kernelTty->scrollbackY = y;
	semSignal(&kernelTty->sem);
	releaseSpinlock(&kernelTty->earlyLock);
}

void ttySwitch(unsigned int ttynr) {
	struct Vtty *tty = &ttys[ttynr];
	struct Vtty *oldCurrent = currentTty;
	oldCurrent->focus = false;
	currentTty = tty;
	tty->focus = true;
	semSignal(&oldCurrent->sem);
	//fbUpdate(tty);
}

static void fbUpdateThread(void) {
	while (true) {
		fbUpdate(currentTty);
		semWait(&currentTty->sem);
		//kthreadSleep(17);
	}
}

void fbPanicUpdate(void) {
	ttys[0].focus = true;
	fbUpdate(&ttys[0]);
}

int fbInitDevFiles(void) {
	printk("[TTY] Creating device files...\n");
	struct Inode *devDir = getInodeFromPath(rootDir, "dev");
	for (unsigned int i = 0; i < NROF_VTTYS; i++) {
		ttyName[sizeof(ttyName) - 2] = '0' + i;
		fsCreateCharDev(devDir, ttyName, &ttyOps, &ttys[i]);
	}
	return 0;
}
MODULE_INIT(fbInitDevFiles);

int fbInitLate(void) {
	ttyEarly = false;
	thread_t updateThread;
	int error = kthreadCreate(&updateThread, (void *(*)(void *))fbUpdateThread, NULL, THREAD_FLAG_DETACHED);
	if (error) {
		return error;
	}
	return 0;
}
MODULE_INIT_LEVEL(fbInitLate, 2);

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
	if (!bootInfo.fbIsRgb) {
		bootFB->colorInfo.rShift = bootInfo.fbRShift;
		bootFB->colorInfo.rSize = bootInfo.fbRSize;
		bootFB->colorInfo.gShift = bootInfo.fbGShift;
		bootFB->colorInfo.gSize = bootInfo.fbGSize;
		bootFB->colorInfo.bShift = bootInfo.fbBShift;
		bootFB->colorInfo.bSize = bootInfo.fbBSize;
	}

	for (unsigned int i = 0; i < NROF_VTTYS; i++) {
		ttys[i].fb = bootFB;
		ttys[i].charWidth = bootFB->xResolution / 8;
		ttys[i].charHeight = bootFB->yResolution / FONT_HEIGHT;
		ttys[i].scrollback = allocKPages(SCROLLBACK_LINES * ttys[i].charWidth,
			PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		ttys[i].cursorX = 0;
		ttys[i].cursorY = 0;
	}
	kernelTty->focus = true;
	setKernelStdout(kernelPuts);

	return 0;
}
MODULE_INIT_LEVEL(fbInit, 0);