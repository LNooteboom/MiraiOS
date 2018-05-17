BITS 64

extern main
extern _init
extern _fini
extern __PHMallocInit
extern stdin
extern stdout
extern stderr
extern _PHCloseAll

global _start:function
global exit:function
global _exit:function
global _Exit:function

global environ:data
global errno:data

global _PHSigTramp:function

SECTION .text

_start:
	;handle args
	mov rdi, [rsp] ;argc
	lea rsi, [rsp + 8] ;argv

	lea rax, [rsi + rdi*8 + 8] ;environ = &argv[argc+1]
	mov [environ], rax

	call __PHMallocInit

	call _init

	pop rdi
	mov rsi, rsp
	mov rax, ~(0xF)
	and rsp, rax
	;sub rsp, 8
	call main

	mov rdi, rax
exit: ;fall through
	push rdi
	cmp qword [_PHAtExitFunc], 0
	je .noAtExit
		call [_PHAtExitFunc]
	.noAtExit:

	call _fini

	call _PHCloseAll
	pop rdi
_exit: ;fall through
_Exit:
	mov eax, 8
	syscall

_PHSigTramp: ;signal trampoline, rax contains sa_handler
	call rax
	mov eax, 0x15
	syscall
	
SECTION .bss

environ: resq 1
_PHAtExitFunc: resq 1

errno: resd 1