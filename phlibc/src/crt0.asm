BITS 64

extern main
extern _init
extern _fini

global _start:function
global exit:function

global environ:data
global errno:data

SECTION .text

_start:
	;handle args
	xchg bx, bx
	mov rdi, [rsp] ;argc2
	lea rsi, [rsp + 8] ;argv

	lea rax, [rsi + rdi*8 + 8] ;environ = &argv[argc+1]
	mov [environ], rax

	call _init

	pop rdi
	mov rsi, rsp
	call main

	mov rdi, rax
	jmp exit

exit:
	push rdi
	cmp qword [_PHAtExitFunc], 0
	je .noAtExit
		call [_PHAtExitFunc]
	.noAtExit:

	call _fini

	pop rdi
	mov eax, 8
	syscall

SECTION .bss

environ: resq 1
_PHAtExitFunc: resq 1

errno: resd 1