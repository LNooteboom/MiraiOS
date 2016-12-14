#ifndef VIDEO_H
#define VIDEO_H

#include <global.h>

//volatile char *vram = (volatile char*)0xB8000;
//extern volatile char *vram;
extern volatile char *vgaMem;
extern size_t vgaMemSize;
extern uint8_t screenWidth;
extern uint16_t screenHeight;

void initVga(void);

void vgaSetCursor(uint16_t cursor);

void vgaSetStart(uint16_t addr);

#endif
