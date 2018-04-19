#ifndef INCLUDE_SCHED_PGRP_H
#define INCLUDE_SCHED_PGRP_H

#include <sched/process.h>
#include <lib/rbtree.h>

//Process groups & sessions

struct PGroup {
	struct RbNode node; //must be first
	struct Session *s;
	pid_t pgid;

	struct PGroup *next;
	struct PGroup *prev;
	struct Process *first;
	struct Process *last;
};

struct Session {
	//spinlock_t lock;
	pid_t sid;

	struct Session *next;
	struct Session *prev;
	struct PGroup *first;
	struct PGroup *last;
};

void leaveGroup(struct Process *proc);
int setpgid(struct Process *proc, pid_t pgid);
struct PGroup *getPGroup(pid_t pgid);

#endif