#ifndef __PHLIBC_SETJMP_H
#define __PHLIBC_SETJMP_H

typedef int jmp_buf[8];

int setjmp(jmp_buf buf);
int _setjmp(jmp_buf buf);

void longjmp(jmp_buf buf, int i) __attribute__((noreturn));
void _longjmp(jmp_buf buf, int i) __attribute__((noreturn));

#endif