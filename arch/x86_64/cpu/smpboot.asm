global smpbootStart:function

global smpboot16start:data
global smpboot16end:data

SECTION .text

smpbootStart:
	xchg bx, bx
	mov rax, 0xDEADBEEFCAFEBABE
	jmp $

SECTION .rodata

smpboot16start:
INCBIN "smpboot16.bin"
smpboot16end: