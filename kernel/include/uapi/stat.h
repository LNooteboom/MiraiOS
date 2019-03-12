#ifndef __PHLIBC_STAT_H
#define __PHLIBC_STAT_H

#include <stdint.h>

struct stat {
	uint32_t st_dev;
	uint32_t st_ino;
	uint32_t st_mode;
	uint32_t st_nlink;
	uint32_t st_uid;
	uint32_t st_gid;
	uint32_t st_rdev;
	uint64_t st_size;
	uint64_t st_blksize;
	uint64_t st_blocks;

	//TODO create/modify/access time
};

#endif