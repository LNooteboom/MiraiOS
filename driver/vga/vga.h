#ifndef VIDEO_H
#define VIDEO_H

//volatile char *vram = (volatile char*)0xB8000;
extern volatile char *vram;
extern uint16_t scrollY;
extern uint32_t screenWidth;
extern uint32_t screenHeight;

void initVga(void);

int vgaGetScreenWidth(void);

int vgaGetScreenHeight(void);

void vgaSetCursor(int cursorX, int cursorY);

void vgaSetScroll(int scrollY);

#endif
