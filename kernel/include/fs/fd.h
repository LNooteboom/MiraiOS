#ifndef INCLUDE_FS_FD_H
#define INCLUDE_FS_FD_H

#include <sched/process.h>

struct Pipe;
struct File;

union FilePipe {
	struct File *file;
	struct Pipe *pipe;
};

int getFileFromFD(struct Process *proc, int fd, union FilePipe *fp);

int allocateFD(struct Process *proc, struct ProcessFile **pf);

#endif