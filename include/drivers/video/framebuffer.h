#ifndef VIDEO_FRAMEBUFFER_H
#define VIDEO_FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <console.h>


struct fbColorInfo {
	uint8_t rSize;
	uint8_t rShift;
	uint8_t gSize;
	uint8_t gShift;
	uint8_t bSize;
	uint8_t bShift;
};

struct framebuffer {
	struct console con;
	unsigned int cursorX;
	unsigned int cursorY;

	char *vmem;
	unsigned int pitch;
	unsigned int xResolution;
	unsigned int yResolution;
	uint32_t bpp;

	bool isRGB;
	struct fbColorInfo colorInfo;
};

#endif