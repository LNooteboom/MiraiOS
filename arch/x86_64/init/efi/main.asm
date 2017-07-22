BITS 64
DEFAULT REL

global bootInfo:data
global __init:function

global initStackEnd:data
global excStackStart:data

global PML4T:data
global PDPT:data
global nxEnabled:data

SECTION .setup align=16 exec

__init:
	mov rcx, [rdx + 64]
	lea rdx, [rel hello2]
	call [rcx + 8]

	jmp $

hello2:
	db __utf16__ `Hello world from kernel!\n\r\0`


SECTION .data
bootInfo: dq 0

nxEnabled: db 0

SECTION .bss align=4096 nobits
PML4T:
	resb 0x1000
PDPT:
	resb 0x1000
PDT:
	resb 0x1000
;Use PSE for kernel static memory
excStackStart:
	resb 0x1000
initStackEnd:
multiBootInfo:
	resq 1