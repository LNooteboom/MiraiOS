NROF_DEFINED_EXCS: equ 21

extern routeInterrupt
extern sprint
extern hexprintln
extern undefinedInterrupt

global initExceptions:function

%define endl 10, 0

SECTION .text

initExceptions:
	push ebp
	mov ebp, esp
	;xchg bx, bx
	push ebx
	push esi

	;push 1
	;push 0
	;push excDivError
	;call routeInterrupt
	;add esp, 0xC

	xor ebx, ebx
	mov esi, excList

	.start:
		cmp ebx, NROF_DEFINED_EXCS
		jae .end
		push 1
		push ebx
		mov eax, [esi]
		push eax
		call routeInterrupt
		add esp, 0xC
		add esi, 4
		inc ebx
		jmp .start

	.end:

	mov esi, [ebp-8]
	mov ebx, [ebp-4]
	leave
	ret

exceptionBase:
	;eax = name
	push ebp
	mov ebp, esp

	push eax
	call sprint
	add esp, 4

	push addressText
	call sprint
	mov eax, [ebp+4]
	push eax
	call hexprintln
	add esp, 8

	.halt:
	cli
	hlt
	jmp .halt

exceptionBaseWithErrorCode:
	;eax = name
	push ebp
	mov ebp, esp

	push eax
	call sprint
	add esp, 4

	push addressText
	call sprint
	mov eax, [ebp+12]
	push eax
	call hexprintln
	add esp, 8

	push errorCode
	call sprint
	mov eax, [ebp+8]
	push eax
	call hexprintln
	add esp, 8

	.halt:
	cli
	hlt
	jmp .halt

excDE:
	mov eax, DEmsg
	jmp exceptionBase

excDB:
	mov eax, DBmsg
	jmp exceptionBase

excNMI:
	iret

excBP:
	mov eax, BPmsg
	jmp exceptionBase

excOF:
	mov eax, OFmsg
	jmp exceptionBase

excBR:
	mov eax, BRmsg
	jmp exceptionBase

excUD:
	mov eax, UDmsg
	jmp exceptionBase

excNM:
	mov eax, NMmsg
	jmp exceptionBase

excDF:
	jmp $
	add esp, 4 ;skip over error code, as it is always zero
	mov eax, DFmsg 
	jmp exceptionBase

excCSO:
	mov eax, CSOmsg
	jmp exceptionBase

excTS:
	mov eax, TSmsg
	jmp exceptionBaseWithErrorCode

excNP:
	mov eax, NPmsg
	jmp exceptionBaseWithErrorCode

excSS:
	mov eax, SSmsg
	jmp exceptionBaseWithErrorCode

excGP:
	mov eax, GPmsg
	jmp exceptionBaseWithErrorCode

excPF:	push ebp
		mov ebp, esp
		;xchg bx, bx
		push eax
		push ecx
		push edx

		push PFmsg
		call sprint
		add esp, 4

		;address
		push addressText
		call sprint
		mov eax, [ebp+8]
		push eax
		call hexprintln
		add esp, 8

		;page
		push PFAddr
		call sprint
		mov eax, cr2
		push eax
		call hexprintln
		add esp, 8

		;error code
		push errorCode
		call sprint
		mov eax, [ebp+4]
		push eax
		call hexprintln
		add esp, 8

		mov edx, [ebp-12]
		mov ecx, [ebp-8]
		mov eax, [ebp-4]
		jmp $
		leave
		add esp, 4 ;skip over error code
		iret

excMF:
	mov eax, MFmsg
	jmp exceptionBase

excAC:
	mov eax, ACmsg
	jmp exceptionBaseWithErrorCode

excMC:
	mov eax, MCmsg
	jmp exceptionBase

excXM:
	mov eax, XMmsg
	jmp exceptionBase

excVE:
	mov eax, VEmsg
	jmp exceptionBase

SECTION .rodata
addressText: db 'At: ', 0
PFAddr: db 'Attempted to access ', 0
errorCode: db 'Error code: ', 0

excList:
dd excDE
dd excDB
dd excNMI
dd excBP
dd excOF
dd excBR
dd excUD
dd excNM
dd excDF
dd excCSO
dd excTS
dd excNP
dd excSS
dd excGP
dd excPF
dd undefinedInterrupt
dd excMF
dd excAC
dd excMC
dd excXM
dd excVE

DEmsg:	db 'Division error', endl
DBmsg:	db 'Debug error', endl
BPmsg:	db 'Breakpoint reached', endl
OFmsg:	db 'Overflow', endl
BRmsg:	db 'BOUND Range exceeded', endl
UDmsg:	db 'Invalid opcode detected', endl
NMmsg:	db 'Coprocessor not available', endl
DFmsg:	db 'Double Fault', endl
CSOmsg:	db 'Coprocessor segment overrun', endl
TSmsg:	db 'Invalid TSS', endl
NPmsg:	db 'Segment not present', endl
SSmsg:	db 'Stack fault', endl
GPmsg:	db 'General protection fault', endl
PFmsg:	db 'Page fault', endl
MFmsg:	db 'Coprocessor error', endl
ACmsg:	db 'Alignment check', endl
MCmsg:	db 'Machine check', endl
XMmsg:	db 'SIMD floating point error', endl
VEmsg:	db 'Virtualization exception', endl
