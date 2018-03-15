#ifndef __PHLIBC_UAPI_GETDENT_H
#define __PHLIBC_UAPI_GETDENT_H

#include <stdint.h>

#define NAME_MAX 256

struct GetDent {
	uint32_t inodeID;
	unsigned int type;
	char name[256];
};

#endif