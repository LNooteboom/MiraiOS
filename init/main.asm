BITS 64

global _start:function

SECTION .text

_start:
mov eax, 3
mov rdi, ttyName
mov esi, 2
syscall ;sysOpen

mov eax, 7
syscall ;sysFork
test rax, rax
jnz .parent
	
	mov eax, 1
	mov edi, 0
	mov rsi, teststr2
	mov edx, 13
	syscall ;sysWrite

	mov eax, 6
	mov rdi, fileName
	syscall ;sysExec
.parent:

mov eax, 9
mov edi, 0
mov esi, 0
mov edx, 0
syscall ;sysWaitPid

mov eax, 1
mov edi, 0
mov rsi, teststr
mov edx, 7 ;14
syscall ;sysWrite
jmp $

SECTION .rodata

rootName:
db '/', 0

ttyName:
db '/dev/tty0', 0

fileName:
db 'test', 0

teststr:
;db 'Parent hello!', 10, 0
db 'files: ', 10, 0
teststr2:
db 'Child hello!', 10, 0

SECTION .bss

buf: resb 512