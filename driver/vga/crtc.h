#ifndef VIDEO_H
#define VIDEO_H

#include <global.h>

//volatile char *vram = (volatile char*)0xB8000;
extern volatile char *vram;
extern uint16_t scroll;
extern uint8_t screenWidth;
extern uint16_t screenHeight;

void initVga(void);

uint8_t vgaGetScreenWidth(void);

uint16_t vgaGetScreenHeight(void);

void vgaSetCursor(uint16_t cursor);

void vgaSetStart(uint16_t addr);

#endif
