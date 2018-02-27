BITS 64

extern main
global _start:function

SECTION .text

_start:
	call main

	mov rdi, rax
	mov eax, 8
	syscall

	jmp $