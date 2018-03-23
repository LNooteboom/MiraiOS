#include "tty.h"
#include <stdbool.h>
#include <errno.h>
#include <mm/paging.h>
#include <uapi/eventcodes.h>
#include <fs/devfile.h>

extern char keymap[];
extern char shiftKeymap[];

static bool shiftPressed = false;

static void ttySignalInputSem(struct Vtty *tty) {
	if (tty->inputAvail.value <= 0) {
		semSignal(&tty->inputAvail);
	}
}

static char backspaceStr[] = "\e[D \e[D";

ssize_t ttyRead(struct File *file, void *buffer, size_t bufSize) {
	ssize_t ret = 0;
	struct Vtty *tty = file->inode->cachedData;

	char *cbuf = (char *)buffer;
	
	semWait(&tty->inputAvail);
	acquireSpinlock(&tty->inputLock);

	int ri = tty->inputReadIndex;
	int end = (tty->inputMode & INPUT_MODE_LINEBUF)? tty->inputLineIndex : ((tty->inputWriteIndex - 1) % INPUT_BUF_SIZE);
	do {
		if (!bufSize) {
			ttySignalInputSem(tty);
			break;
		}
		ri = (ri + 1) % INPUT_BUF_SIZE;
		*cbuf++ = tty->inputBuf[ri];
		bufSize--;
		ret++;
	} while (ri != end);

	tty->inputReadIndex = ri;
	releaseSpinlock(&tty->inputLock);
	return ret;
}

int ttyInitInput(void) {
	int error = 0;
	for (int i = 0; i < NROF_VTTYS; i++) {
		ttys[i].inputBuf = allocKPages(INPUT_BUF_SIZE, PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		if (!ttys[i].inputBuf) {
			error = -ENOMEM;
			goto ret;
		}
		ttys[i].inputReadIndex = INPUT_BUF_SIZE - 1;
		ttys[i].inputLineIndex = INPUT_BUF_SIZE - 1;
		ttys[i].inputMode = INPUT_MODE_LINEBUF | INPUT_MODE_ECHO;
	}

	ret:
	return error;
}

int ttyHandleKeyEvent(int eventCode, bool released) {
	if (released) {
		if (eventCode == KEY_LEFTSHIFT) {
			shiftPressed = false;
		}
		return 0;
	}
	if (eventCode == KEY_LEFTSHIFT) {
		shiftPressed = true;
		return 0;
	} else if (eventCode >= KEY_F1 && eventCode <= KEY_F8) {
		ttySwitch(eventCode - KEY_F1);
		return 0;
	} else if (eventCode == KEY_PAGEUP) {
		ttyScroll(-1);
		return 0;
	} else if (eventCode == KEY_PAGEDOWN) {
		ttyScroll(1);
		return 0;
	}
	char c = (shiftPressed)? shiftKeymap[eventCode] : keymap[eventCode];
	if (!c) {
		return -EINVAL;
	}
	
	//push c to inputbuf
	struct Vtty *tty = currentTty;
	acquireSpinlock(&tty->inputLock);

	int wi = tty->inputWriteIndex;
	int echo = tty->inputMode & INPUT_MODE_ECHO;
	if (c == '\n' && tty->inputMode & INPUT_MODE_LINEBUF) {
		tty->inputLineIndex = wi;
		ttySignalInputSem(tty);
	} else if (c == 127 && tty->inputMode & INPUT_MODE_LINEBUF) {
		//backspace
		int wi2 = (wi - 1) & (INPUT_BUF_SIZE - 1);
		if (wi2 != tty->inputLineIndex) {
			tty->inputWriteIndex = wi2;
			releaseSpinlock(&tty->inputLock);
			if (echo) {
				ttyPuts(tty, backspaceStr, sizeof(backspaceStr) - 1);
			}
		} else {
			releaseSpinlock(&tty->inputLock);
		}
		return 0;
	} else if (!(tty->inputMode & INPUT_MODE_LINEBUF)) {
		ttySignalInputSem(tty);
	}
	
	tty->inputBuf[wi] = c;

	if (wi == tty->inputReadIndex) {
		tty->inputReadIndex = (tty->inputReadIndex + 1) % INPUT_BUF_SIZE;
	}/* else if (tty->inputReadIndex == -1) {
		tty->inputReadIndex = 0;
	}*/

	wi = (wi + 1) % INPUT_BUF_SIZE;
	tty->inputWriteIndex = wi;

	releaseSpinlock(&tty->inputLock);
	if (echo) {
		ttyPuts(tty, &c, 1);
	}
	return 0;
}