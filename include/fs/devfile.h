#ifndef INCLUDE_FS_DEVFILE_H
#define INCLUDE_FS_DEVFILE_H

#include <stddef.h>
#include <stdarg.h>
#include <fs/fs.h>

struct DevFileOps {
	ssize_t (*read)(struct File *file, void *buffer, size_t bufSize);
	int (*write)(struct File *file, void *buffer, size_t bufSize);
	int (*ioctl)(struct File *file, unsigned long request, va_list args);
	int (*del)(struct Inode *inode);
};

struct Inode *fsCreateCharDev(struct Inode *dir, const char *name, const struct DevFileOps *ops, void *privateData);

#endif