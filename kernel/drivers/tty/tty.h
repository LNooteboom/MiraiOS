#ifndef TTY_H
#define TTY_H

#include <sched/spinlock.h>
#include <sched/lock.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mm/paging.h>
#include <fs/fs.h>

#define FONT_HEIGHT 16
#define FONT_WIDTH	8
#define SCROLLBACK_LINES 256
#define NROF_VTTYS	8
#define NROF_COLORS	16

#define INPUT_BUF_SIZE		PAGE_SIZE
#define INPUT_MODE_LINEBUF	1
#define INPUT_MODE_ECHO		2
#define INPUT_MODE_NOBLOCK	4

/*struct FbColorInfo { //bootinfo struct
	uint8_t rSize;
	uint8_t rShift;
	uint8_t gSize;
	uint8_t gShift;
	uint8_t bSize;
	uint8_t bShift;
};*/

struct Framebuffer {
	char *vmem; //video memory
	unsigned int pitch;
	unsigned int xResolution;
	unsigned int yResolution;
	uint32_t bpp;

	bool isRGB;
	//struct FbColorInfo colorInfo;
};

struct VttyChar {
	char c;
	uint8_t fgCol;
	uint8_t bgCol;
	uint8_t dirty;
};

struct Vtty {
	//Display-side info
	struct Framebuffer *fb;
	spinlock_t lock;

	int charWidth; //width of the screen in characters
	int charHeight; //height of the screen in characters
	int cursorX;
	int cursorY;
	struct VttyChar *buf;

	uint8_t curFGCol;
	uint8_t curBGCol;

	int scrollbackY;
	bool full;
	bool globalDirty;
	volatile bool focus;

	semaphore_t updateSem;

	//Input info
	spinlock_t inputLock;
	char *inputBuf;
	int inputWriteIndex;
	int inputReadIndex;
	int inputLineIndex;
	int inputMode;
	semaphore_t inputAvail;
};

extern struct Vtty ttys[NROF_VTTYS];
extern struct Vtty *kernelTty;
extern struct Vtty *currentTty;

int ttyPuts(struct Vtty *tty, const char *text, size_t textLen);

ssize_t ttyRead(struct File *file, void *buffer, size_t bufSize);

int ttyInitInput(void);

#endif