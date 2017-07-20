#include <drivers/video/framebuffer.h>

#include <arch/bootinfo.h>
#include <modules.h>
#include <io.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>

#define FONT_HEIGHT 16

extern char font8x16[];

static void putPix(struct framebuffer *fb, char *addr, uint32_t col) {
	if (fb->isRGB) {
		write32(addr, col);
	}
}

static int fbPutc(struct console *con, char c) {
	struct framebuffer *fb = (struct framebuffer*)con;
	if (c >= 32 && c <= 126) {
		unsigned int index = (c - 32) * FONT_HEIGHT;
		char *pos = fb->vmem + fb->cursorX + (fb->cursorY * fb->pitch);
		size_t newline = fb->pitch - (fb->bpp * 8);
		for (unsigned int y = 0; y < FONT_HEIGHT; y++) {
			char line = font8x16[index++];
			for (unsigned int x = 0; x < 8; x++) {
				if (line & 1) {
					putPix(fb, pos, ~0);
				} else {
					putPix(fb, pos, 0);
				}
				pos += fb->bpp;
				line >>= 1;
			}
			pos += newline;
		}

		fb->cursorX += fb->bpp * 8;
	} else if (c == '\n') {
		fb->cursorX = 0;
		fb->cursorY += FONT_HEIGHT;
	}
	return 0;
}

int fbInit(void) {
	//check if bootfb exists
	if (!bootInfo.fbAddr)
		return -EIO;
	char *vmem = ioremap(bootInfo.fbAddr, bootInfo.fbSize);
	struct framebuffer *bootFB = kmalloc(sizeof(struct framebuffer));
	if (!bootFB || !vmem)
		return -ENOMEM;

	bootFB->con.putc = fbPutc;
	bootFB->con.lock = 0;
	bootFB->cursorX = 0;
	bootFB->cursorY = 0;

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
	registerConsole(&bootFB->con);

	return 0;
}

//MODULE_INIT_LEVEL(fbInit, 0);