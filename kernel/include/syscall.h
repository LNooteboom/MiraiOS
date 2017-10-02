#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

/*
#define DEFINE_SYSCALL(func) \
	static int (*syscall_##func)(...) __attribute__((used, section (".syscalls"))) = func
*/

void registerSyscall(int nr, int (*func)());

#endif