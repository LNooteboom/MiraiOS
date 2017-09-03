#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H

#include <sched/spinlock.h>

struct Console {
	int (*putc)(struct Console *con, char c);
	spinlock_t lock;
};

int registerConsole(struct Console *con);

#endif