#include "crtc.h"

#include <stdint.h>
#include <stddef.h>
#include <mm/paging.h>
#include <io.h>

#define REG_MISC_OUTPUT_READ	0x3CC
#define REG_MISC_OUTPUT_WRITE	0x3C2
#define REG_GRAPHICS_INDEX		0x3CE
#define REG_GRAPHICS_DATA		0x3CF

#define CRTC_INDEX_OVERFLOW				0x07
#define CRTC_INDEX_MAX_SCANLINE			0x09
#define CRTC_INDEX_START_ADDRESS_HIGH	0x0C
#define CRTC_INDEX_START_ADDRESS_LOW	0x0D
#define CRTC_INDEX_CURSOR_HIGH			0x0E
#define CRTC_INDEX_CURSOR_LOW			0x0F
#define CRTC_INDEX_VERTICAL_DISPLAY_END	0x12
#define CRTC_INDEX_OFFSET				0x13
#define CRTC_INDEX_CRTC_MODE			0x17

#define GRAPH_INDEX_MISC				0x06

#define getCRTCIndex()			in8(CRTCIndexPort)
#define setCRTCIndex(index)		out8(CRTCIndexPort, index)
#define getCRTCData()			in8(CRTCDataPort)
#define setCRTCData(data)		out8(CRTCDataPort, data)

#define getGraphIndex()			in8(REG_GRAPHICS_INDEX)
#define setGraphIndex(index)	out8(REG_GRAPHICS_INDEX, index)
#define getGraphData()			in8(REG_GRAPHICS_DATA)
#define setGraphData(data)		out8(REG_GRAPHICS_DATA, data)

static unsigned int CRTCIndexPort;
static unsigned int CRTCDataPort;

volatile char *vgaMem;
size_t vgaMemSize;

uint8_t vgaScreenWidth;
uint16_t vgaScreenHeight;

static void getCRTCPorts(void) {
	uint8_t miscOutputReg = in8(REG_MISC_OUTPUT_READ);
	if (miscOutputReg & 1) {
		CRTCIndexPort = 0x3D4;
		CRTCDataPort = 0x3D5;
	} else {
		CRTCIndexPort = 0x3B4;
		CRTCDataPort = 0x3B5;
	}
}

static void getVramAddr(void) {
	uint8_t oldIndex = getGraphIndex();

	setGraphIndex(GRAPH_INDEX_MISC);
	uint8_t mmapSel = getGraphData() >> 2;
	vgaMem = &VMEM_OFFSET;
	switch (mmapSel) {
		case 0:
			vgaMem += 0xA0000;
			vgaMemSize = 0x20000;
			break;
		case 1:
			vgaMem += 0xA0000;
			vgaMemSize = 0x10000;
			break;
		case 2:
			vgaMem += 0xB0000;
			vgaMemSize = 0x8000;
			break;
		default: //case 3:
			vgaMem += 0xB8000;
			vgaMemSize = 0x8000;
			break;
	}

	setGraphIndex(oldIndex);
}

static uint8_t getScreenWidth(void) {
	uint8_t oldIndex = getCRTCIndex();
	
	setCRTCIndex(CRTC_INDEX_OFFSET);
	uint8_t offset = getCRTCData();

	setCRTCIndex(oldIndex);
	return offset * 2;
}

static uint16_t getScreenHeight(void) {
	uint8_t oldIndex = getCRTCIndex();

	//Get maximum scanline per char
	setCRTCIndex(CRTC_INDEX_MAX_SCANLINE);
	uint8_t maxScanLine = (getCRTCData() & 0x1F) + 1;

	//Get number of scanlines in active display
	setCRTCIndex(CRTC_INDEX_VERTICAL_DISPLAY_END);
	uint16_t vDispEnd = getCRTCData();
	//Get bit 8 and 9
	setCRTCIndex(CRTC_INDEX_OVERFLOW);
	uint8_t overflow = getCRTCData();
	if (overflow & 0x02) {
		vDispEnd |= (1 << 8);
	}
	if (overflow & 0x40) {
		vDispEnd |= (1 << 9);
	}

	setCRTCIndex(oldIndex);

	return vDispEnd / maxScanLine;
}

void vgaInit(void) {
	getCRTCPorts();
	getVramAddr();
	vgaScreenWidth = getScreenWidth();
	vgaScreenHeight = getScreenHeight();
}

void vgaSetCursor(uint16_t cursor) {
	uint8_t oldIndex = getCRTCIndex();

	setCRTCIndex(CRTC_INDEX_CURSOR_LOW);
	setCRTCData((uint8_t)cursor);
	setCRTCIndex(CRTC_INDEX_CURSOR_HIGH);
	setCRTCData((uint8_t)(cursor >> 8));

	setCRTCIndex(oldIndex);
}

void vgaSetStart(uint16_t addr) {
	uint8_t oldIndex = getCRTCIndex();

	setCRTCIndex(CRTC_INDEX_START_ADDRESS_LOW);
	setCRTCData((uint8_t)addr);
	setCRTCIndex(CRTC_INDEX_START_ADDRESS_HIGH);
	setCRTCData((uint8_t)(addr >> 8));

	setCRTCIndex(oldIndex);
}
