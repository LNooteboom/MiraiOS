BITS 64

global _start:function

_start:
mov rax, 17
syscall
test rax, rax
jnz .parent
	mov rsi, teststr2
	mov edx, 15
	jmp .end
.parent:
	mov rsi, teststr
	mov edx, 14
.end:

mov eax, 1
mov edi, 1
;mov rsi, teststr
;mov edx, 22
syscall

jmp $

teststr:
db 'Parent hello!', 10, 0
teststr2:
db 'Child hello!', 10, 0