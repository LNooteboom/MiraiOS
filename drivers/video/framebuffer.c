#include <arch/bootinfo.h>
#include <modules.h>
#include <io.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <sched/spinlock.h>
#include <sched/thread.h>
#include <sched/sleep.h>
#include <sched/lock.h>
#include <fs/fs.h>
#include <print.h>

#define FONT_HEIGHT 16
#define SCROLLBACK_LINES 256
#define NROF_VTTYS	8

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
	union {
		spinlock_t earlyLock;
		struct Inode *inode;
	};

	uint16_t charWidth; //width of the screen in characters
	uint16_t charHeight; //height of the screen in characters
	uint16_t cursorX;
	uint16_t cursorY;
	char *scrollback;

	uint16_t scrollbackY;
	bool full;
	bool focus;

	semaphore_t sem;
};

extern char font8x16[];

static struct Vtty ttys[NROF_VTTYS];
static struct Vtty *kernelTty = &ttys[0];
static struct Vtty *currentTty = &ttys[0];

bool ttyEarly = true;

static void drawChar(char *vmem, unsigned int newline, char c) {
	if (c < 32 || c > 126) {
		c = ' ';
	}
	unsigned int charIndex = (c - 32) * FONT_HEIGHT;
	for (unsigned int charY = 0; charY < FONT_HEIGHT; charY++) {
		char line = font8x16[charIndex++];
		for (unsigned int charX = 0; charX < 8; charX++) {
			uint32_t color = 0;
			if (line & 1) {
				color = 0xAAAAAA;
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

static int kernelPutsEarly(const char *text) {
	struct Vtty *tty = kernelTty;
	acquireSpinlock(&tty->earlyLock);

	while (*text) {
		switch (*text) {
			case '\r':
				tty->cursorX = 0;
				break;
			case '\n':
				newline(tty);
				break;
			default:
				tty->scrollback[tty->cursorY * tty->charWidth + tty->cursorX] = *text;
				tty->cursorX++;
				if (tty->cursorX >= tty->charWidth) {
					newline(tty);
				}
				break;
		}
		text++;
	}
	if (ttyEarly) {
		fbUpdate(tty);
	} else {
		//tty->dirty = true;
		semSignal(&tty->sem);
	}

	releaseSpinlock(&tty->earlyLock);
	return 0;
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
	}
}

int fbInitLate(void) {
	ttyEarly = false;
	int error = kthreadCreate(NULL, (void *(*)(void *))fbUpdateThread, NULL, THREAD_FLAG_DETACHED);
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
			PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		ttys[i].cursorX = 0;
		ttys[i].cursorY = 0;
	}
	kernelTty->focus = true;
	setKernelStdout(kernelPutsEarly);

	return 0;
}
MODULE_INIT_LEVEL(fbInit, 0);