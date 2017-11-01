BITS 64

global _start:function

_start:
mov rax, 17
syscall

mov eax, 1
mov edi, 1
mov rsi, teststr
mov edx, 22
syscall

jmp $

teststr:
db 'Hello from userspace!', 10, 0