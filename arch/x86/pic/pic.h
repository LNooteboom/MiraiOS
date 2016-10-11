#ifndef IO_H
#define IO_H

char inb(short port);
void outb(short port, char value);

void initPICS(void);

char pic_getmask_master(void);
void pic_setmask_master(char mask);

#endif
