#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H

#include <sched/spinlock.h>

struct console {
	int (*putc)(struct console *con, char c);
	spinlock_t lock;
};

int registerConsole(struct console *con);

#endif