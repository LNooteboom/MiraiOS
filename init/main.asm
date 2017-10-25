BITS 64

global _start:function

_start:
;ud2
nop

mov eax, 1
mov edi, 1
mov rsi, teststr
mov edx, 21
syscall

jmp $

teststr:
db 'Hello from userspace!', 0