BITS 64

global _start:function

SECTION .text

_start:
mov eax, 17
syscall
test rax, rax
jnz .parent
	
	mov eax, 1
	mov edi, 1
	mov rsi, teststr2
	mov edx, 13
	syscall

	mov eax, 16
	mov rdi, fileName
	syscall
.parent:
mov eax, 1
mov edi, 1
mov rsi, teststr
mov edx, 14
syscall
nop

jmp $

SECTION .rodata

fileName:
db 'test', 0

teststr:
db 'Parent hello!', 10, 0
teststr2:
db 'Child hello!', 10, 0