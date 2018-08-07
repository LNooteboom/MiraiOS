#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <uapi/syscalls.h>
#include <uapi/getDent.h>
#include <uapi/fcntl.h>

DIR *opendir(const char *dirName) {
	DIR *dir = malloc(sizeof(DIR));
	if (!dir) {
		errno = ENOMEM;
		return NULL;
	}

	int dirFD = sysOpen(AT_FDCWD, dirName, SYSOPEN_FLAG_READ | SYSOPEN_FLAG_DIR);
	if (dirFD < 0) {
		errno = -dirFD;
		return NULL;
	}
	dir->dirFD = dirFD;
	return dir;
}

struct dirent *readdir(DIR *dirp) {
	//will assume struct GetDent and struct dirent are the same
	int err = sysGetDent(dirp->dirFD, (struct GetDent *)&dirp->entry);
	if (err < 0) {
		errno = -err;
		return NULL;
	}
	if (err == 0) {
		return NULL; //end of directory
	}
	return &dirp->entry;
}

int closedir(DIR *dirp) {
	sysClose(dirp->dirFD);
	free(dirp);
	return 0;
}

void rewinddir(DIR *dirp) {
	sysSeek(dirp->dirFD, 0, SEEK_SET);
}