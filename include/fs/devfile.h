#ifndef INCLUDE_FS_DEVFILE_H
#define INCLUDE_FS_DEVFILE_H

#include <stddef.h>
#include <stdarg.h>
#include <fs/fs.h>

struct devFileOps {
	ssize_t (*read)(struct file *file, void *buffer, size_t bufSize);
	int (*write)(struct file *file, void *buffer, size_t bufSize);
	int (*ioctl)(struct file *file, unsigned long request, va_list args);
	int (*del)(struct inode *inode);
};

int fsCreateCharDev(struct inode *dir, const char *name, struct devFileOps *ops);

#endif