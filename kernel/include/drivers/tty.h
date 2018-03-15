#ifndef INCLUDE_DRIVERS_TTY_H
#define INCLUDE_DRIVERS_TTY_H

void ttySwitch(unsigned int ttynr);

void ttyScroll(int amount);

int ttyHandleKeyEvent(int eventCode, bool released);

#endif