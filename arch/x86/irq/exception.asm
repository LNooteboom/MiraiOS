NROF_DEFINED_EXCS: equ 21

extern routeInterrupt
extern sprint
extern hexprintln
extern undefinedInterrupt

global initExceptions:function

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

excDE:	;mov eax, 0xdeadbeef
		;push ebp
		;mov ebp, esp
		push diverrormsg
		call sprint
		add esp, 4
		jmp $
		;iret

excDB:
excNMI:
excBP:
excOF:
excBR:
excUD:
excNM:
excDF:
excCSO:
excTS:
excNP:
excSS:
excGP:
		;mov eax, 0xDEADBEEF
		;jmp $
		iret

excPF:	push ebp
		mov ebp, esp
		;xchg bx, bx
		push eax
		push ecx
		push edx

		push pagefault
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
excAC:
excMC:
excXM:
excVE:
		;jmp $
		iret

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

diverrormsg:	db `Division error\n\0`
dbgerror:		db `Debug error\n\0`
breakpointerr:	db `Breakpoint reached\n\0`
overflowerr:	db `Overflow\n\0`
bndrangeerr:	db `BOUND Range exceeded\n\0`
invopcodeerr:	db `Invalid opcode detected\n\0`
coprocnavail:	db `Coprocessor not available\n\0`
coprocsegoverrun:db `Coprocessor segment overrun\n\0`
invalidTSSerr:	db `Invalid TSS\n\0`
segnpresent:	db `Segment not present\n\0`
stackfault:		db `Stack fault\n\0`
genprot:		db `General protection fault\n\0`
pagefault:		db `Page fault\n\0`
coprocerr:		db `Coprocessor error\n\0`
dblfault:		db `Double Fault\n\0`

