#ifndef INCLUDE_PARAM_MAIN_H
#define INCLUDE_PARAM_MAIN_H

#include <global.h>
#include "bootinfo.h"
#include "mmap.h"

extern struct mmap *paramMmap;

/*
This function makes all the parameters in bootInfo ready to use, and converts the physical addresses inside them to virtual addresses.
*/
void paramInit(void);

#endif
