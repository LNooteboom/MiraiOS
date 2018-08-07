#ifndef __PHLIBC_DIRENT_H
#define __PHLIBC_DIRENT_H

#include <phlibc/intsizes.h>

#define DT_DIR	1
#define DT_REG	2
#define DT_CHAR	3

#ifndef __PHLIBC_DEF_INO_T
#define __PHLIBC_DEF_INO_T
typedef __PHLIBC_TYPE_INO_T ino_t;
#endif

struct dirent {
	ino_t d_ino;
	unsigned int d_type;
	char d_name[256];
};

typedef struct {
	int dirFD;
	struct dirent entry;
} DIR;

#if defined(__cplusplus)
extern "C" {
#endif

DIR *opendir(const char *dirName);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif