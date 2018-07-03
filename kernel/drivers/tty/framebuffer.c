#include "tty.h"

#include <arch/bootinfo.h>
#include <modules.h>
#include <io.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/memset.h>
#include <sched/thread.h>
#include <fs/fs.h>
#include <fs/devfile.h>
#include <print.h>
#include <userspace.h>

#include <arch/map.h>

extern char font8x16[];

static bool ttyEarly = true;

#define BPP 4

static uint32_t colors[NROF_COLORS] = {
	0x000000,
	0x0000AA,
	0x00AA00,
	0x00AAAA,
	0xAA0000,
	0xAA00AA,
	0xAA5500,
	0xAAAAAA,
	0x555555,
	0x5555FF,
	0x55FF55,
	0x55FFFF,
	0xFF5555,
	0xFF55FF,
	0xFFFFFF,
	0xFFFFFF
};

static void drawChar(char *vmem, int newline, struct VttyChar *vc) {
	char c = vc->c;
	if (c < 32 || c > 126) {
		c = ' ';
	}

	uint32_t fgPix = colors[vc->fgCol];
	//uint32_t fgPix = 0xFF;
	uint32_t bgPix = colors[vc->bgCol];

	unsigned int charIndex = (c - 32) * FONT_HEIGHT;
	for (unsigned int charY = 0; charY < FONT_HEIGHT; charY++) {
		unsigned char line = font8x16[charIndex++];
		for (unsigned int charX = 0; charX < 8; charX++) {
			if (line & 1) {
				write32(vmem, fgPix);
			} else {
				write32(vmem, bgPix);
			}
			vmem += BPP;
			line >>= 1;
		}
		vmem += newline;
	}
}

static int fbUpdate(struct Vtty *tty) {
	if (!tty->focus) return 0;
	struct Framebuffer *fb = tty->fb;

	int newline = fb->pitch - (FONT_WIDTH * BPP);

	char *lineAddr = fb->vmem;
	char *addr;
	struct VttyChar *vc = tty->buf + tty->scrollbackY * tty->charWidth;

	for (int y = 0; y < tty->charHeight; y++) {
		addr = lineAddr;
		for (int x = 0; x < tty->charWidth; x++) {
			if (vc->dirty || tty->globalDirty) {
				drawChar(addr, newline, vc);
				vc->dirty = 0;
			}
			vc++;
			addr += (FONT_WIDTH * BPP);
		}
		lineAddr += fb->pitch * FONT_HEIGHT;
	}
	tty->globalDirty = false;

	//draw cursor
	vc = tty->buf + tty->cursorX + tty->cursorY * tty->charWidth;
	addr = fb->vmem + fb->pitch * FONT_HEIGHT * (tty->cursorY - tty->scrollbackY) + tty->cursorX * FONT_WIDTH * BPP;
	struct VttyChar curVc = {
		.c = vc->c,
		.bgCol = 7,
		.fgCol = 0
	};
	drawChar(addr, newline, &curVc);

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
	/*struct VttyChar *vc = &tty->buf[tty->cursorY * tty->charWidth];
	for (int x = 0; x < tty->charWidth; x++) {
		vc->c = 0;
		vc->bgCol = 0;
		vc->dirty = 1;
		vc++;
	}*/

	//adjust scrollback pos
	int y = tty->cursorY - tty->charHeight + 1;
	if (y < 0) {
		if (tty->full) {
			y = SCROLLBACK_LINES + y;
		} else {
			y = 0;
		}
	} else if (y >= SCROLLBACK_LINES) {
		y = y % SCROLLBACK_LINES;
	}
	if (tty->scrollbackY != y) {
		tty->scrollbackY = y;
		tty->globalDirty = true;
	}
}

//swap bit 0 and 2
static uint8_t sgrToColor(int sgr) {
	uint8_t ret = sgr & 0xA;
	ret |= (sgr & 1) << 2;
	ret |= (sgr & 4) >> 2;
	return ret;
}

//Returns nrof chars to skip
static int parseEscape(struct Vtty *tty, const char *text) {
	int final = 0;
	struct UserAccBuf b;
	USER_ACC_TRY(b, final) {

	if (text[1] != '[') {
		goto ret;
	}
	int args[8];
	memset(args, 0, 8 * sizeof(int));
	int nrofArgs = 0;
	int i = 2;
	bool noArgs = true;

	while (text[i]) {
		if (text[i] >= '0' && text[i] <= '9') {
			noArgs = false;
			args[nrofArgs] *= 10;
			args[nrofArgs] += text[i] - '0';
		} else if (text[i] == ';' && nrofArgs < 7) {
			nrofArgs++;
		} else if (text[i] >= 0x40 && text[i] <= 0x7E) {
			final = i;
			break;
		}
		i++;
	}
	if (!final) {
		goto ret;
	}
	if (!noArgs) {
		nrofArgs++;
	}

	switch (text[final]) {
		case 'D': //Cursor back
			if (tty->cursorX) {
				tty->cursorX--;
			} else {
				tty->cursorY = (tty->cursorY - 1) % SCROLLBACK_LINES;
				tty->cursorX = tty->charWidth - 1;
			}
			tty->buf[tty->cursorX + tty->cursorY * tty->charWidth].dirty = 1;
			break;
		case 'm': //Set graphic rendition
			for (i = 0; i < nrofArgs; i++) {
				if (!args[i]) { //reset
					tty->curFGCol = 7;
					tty->curBGCol = 0;
				} else if (args[i] == 1) { //toggle bold
					tty->curFGCol ^= 8;
				} else if (args[i] >= 30 && args[i] <= 37) { //fg color
					tty->curFGCol = sgrToColor(args[i] - 30);
				} else if (args[i] >= 40 && args[i] <= 47) { //bg color
					tty->curBGCol = sgrToColor(args[i] - 40);
				} else if (args[i] >= 90 && args[i] <= 97) { //fg bright color
					tty->curFGCol = sgrToColor(args[i] - 90) + 8;
				} else if (args[i] >= 100 && args[i] <= 107) { //bg bright color
					tty->curBGCol = sgrToColor(args[i] - 100) + 8;
				}
			}
			break;
	}
	ret:
	USER_ACC_END();}
	return final;
}

int ttyPuts(struct Vtty *tty, const char *text, size_t textLen) {
	acquireSpinlock(&tty->lock);
	int linePos = tty->cursorY * tty->charWidth;
	struct VttyChar *vc;
	int error = 0;
	for (unsigned int i = 0; i < textLen; i++) {
		tty->buf[linePos + tty->cursorX].dirty = 1;
		char c;
		if (ttyEarly) {
			c = text[i];
		} else {
			struct UserAccBuf b;
			USER_ACC_TRY(b, error) {
				c = text[i];
				USER_ACC_END();
			} USER_ACC_CATCH {
				goto upd;
			}
		}
		switch (c) {
			case '\r':
				tty->cursorX = 0;
				break;
			case '\n':
				newline(tty);
				linePos = tty->cursorY * tty->charWidth;
				break;
			case '\e':
				i += parseEscape(tty, &text[i]);
				break;
			default:
				vc = &tty->buf[linePos + tty->cursorX];
				vc->c = c;
				vc->fgCol = tty->curFGCol;
				vc->bgCol = tty->curBGCol;
				//vc->dirty = 1;

				tty->cursorX++;
				if (tty->cursorX >= tty->charWidth) {
					newline(tty);
				}
				break;
		}
	}
	upd:
	if (ttyEarly) {
		fbUpdate(tty);
		releaseSpinlock(&tty->lock);
	} else {
		releaseSpinlock(&tty->lock);
		if (tty->updateSem.value <= 0) {
			semSignal(&tty->updateSem);
		}
	}
	return error;
}

void ttyScroll(int amount) {
	acquireSpinlock(&currentTty->lock);

	int y = currentTty->scrollbackY + amount;
	if (y < 0) {
		if (currentTty->full) {
			y = SCROLLBACK_LINES + y;
		} else {
			y = 0;
		}
	} else if (y >= SCROLLBACK_LINES) {
		y = y % SCROLLBACK_LINES;
	}
	if (y == (currentTty->cursorY - currentTty->charHeight + 2) % SCROLLBACK_LINES) {
		y = (currentTty->cursorY - currentTty->charHeight + 1) % SCROLLBACK_LINES;
	}
	currentTty->scrollbackY = y;
	currentTty->globalDirty = true;
	semSignal(&currentTty->updateSem);

	releaseSpinlock(&currentTty->lock);
}

void ttySwitch(unsigned int ttynr) {
	if (ttynr >= NROF_VTTYS) return;

	struct Vtty *tty = &ttys[ttynr];
	struct Vtty *oldCurrent = currentTty;
	oldCurrent->focus = false;
	currentTty = tty;
	tty->focus = true;
	tty->globalDirty = true;

	semSignal(&oldCurrent->updateSem);
}

static void fbUpdateThread(void) {
	while (true) {
		acquireSpinlock(&currentTty->lock);
		fbUpdate(currentTty);
		releaseSpinlock(&currentTty->lock);
		semWait(&currentTty->updateSem);
	}
}

void fbPanicUpdate(void) {
	ttys[0].focus = true;
	fbUpdate(&ttys[0]);
}

int fbInitLate(void) {
	ttyEarly = false;
	thread_t updateThread;
	int error = kthreadCreate(&updateThread, (void *(*)(void *))fbUpdateThread, NULL, THREAD_FLAG_DETACHED | THREAD_FLAG_RT);
	if (error) {
		return error;
	}
	return 0;
}
MODULE_INIT_LEVEL(fbInitLate, 2);