#ifndef PS2_H
#define PS2_H

void ps2SendCommand(char command);

char ps2ReadData(void);

void ps2WriteData(char data);

char ps2ReadStatus(void);

void ps2SendPort1(char data);


void ps2Init(void);

void ps2FlushOutput(void);

#endif
