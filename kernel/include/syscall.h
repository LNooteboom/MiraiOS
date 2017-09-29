#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

void registerSyscall(int nr, int (*func)());

#endif