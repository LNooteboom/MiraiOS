extern panic
extern sprint
extern hexprintln

SECTION .text

exc_diverror:	;mov eax, 0xdeadbeef
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4] ;get old eip
		push eax ;and push it
		mov eax, diverrormsg
		push eax
		call panic
		jmp $
		;iret

exc_debug_error:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, dbgerror
		push eax
		call panic
		jmp $
		iret

exc_breakpoint:
		push ebp
		mov ebp, esp
		mov eax, breakpointerr
		push eax
		call sprint
		leave
		iret

exc_overflow:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, overflowerr
		push eax
		call panic
		jmp $

exc_bounds_check:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, bndrangeerr
		push eax
		call panic
		jmp $

exc_inv_opcode:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, invopcodeerr
		push eax
		call panic
		jmp $
		iret

exc_coproc_navail:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, coprocnavail
		push eax
		call panic
		jmp $
		iret

exc_double_fault: ;BSOD time!
		push ebp
		mov esp, ebp
		push 0
		mov eax, [ss:ebp+8]
		push eax
		mov eax, dblfault
		push eax
		call panic
		
		jmp $

exc_coproc_seg_overrun:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, coprocsegoverrun
		push eax
		call panic
		jmp $
		iret

exc_invalid_tss:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, invalidTSSerr
		push eax
		call panic
		jmp $
		iret

exc_seg_npresent:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, segnpresent
		push eax
		call panic
		jmp $
		iret

exc_stack:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, stackfault
		push eax
		call panic
		jmp $
		iret

exc_gen_prot:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, genprot
		push eax
		call panic
		jmp $
		iret

exc_page_fault:
		push ebp
		mov ebp, esp
		push edx
		push ecx
		push ebx
		push eax
		mov eax, [ss:ebp+4]
		push eax
		mov eax, [ss:ebp+8]
		push eax
		mov eax, pagefault
		push eax
		call panic
		add esp, 0x0C

		xor ecx, ecx
		mov ebp, regs
	.start:	;inc ebp
		push ebp
		call sprint
		add esp, 4
		call hexprintln
		add esp, 4
		add ebp, 6
		cmp ebp, regsend
		jb .start
		
		jmp $
		iret

exc_coproc_error:
		push ebp
		mov ebp, esp
		push 0
		mov eax, [ss:ebp+4]
		push eax
		mov eax, coprocerr
		push eax
		call panic
		jmp $
		iret

SECTION .rodata

regs:		db 'eax: ', 0
		db 'ebx: ', 0
		db 'ecx: ', 0
		db 'edx: ', 0
regsend:

diverrormsg:	db 'Division error',0
dbgerror:	db 'Debug error', 0
breakpointerr:	db 'Breakpoint reached', 0
overflowerr:	db 'Overflow', 0
bndrangeerr:	db 'BOUND Range exceeded', 0
invopcodeerr:	db 'Invalid opcode detected', 0
coprocnavail:	db 'Coprocessor not available', 0
coprocsegoverrun:db 'Coprocessor segment overrun', 0
invalidTSSerr:	db 'Invalid TSS', 0
segnpresent:	db 'Segment not present', 0
stackfault:	db 'Stack fault', 0
genprot:	db 'General protection fault', 0
pagefault:	db 'Page fault', 0
coprocerr:	db 'Coprocessor error', 0
dblfault:	db 'Double Fault', 0

excList:
		dd exc_diverror
		dd exc_debug_error
		;dd irq_undefined ;NMI
		dd exc_breakpoint
		dd exc_overflow
		dd exc_bounds_check
		dd exc_inv_opcode
		dd exc_coproc_navail
		dd exc_double_fault
		dd exc_coproc_seg_overrun
		dd exc_invalid_tss
		dd exc_seg_npresent
		dd exc_stack
		dd exc_gen_prot
		dd exc_page_fault
		;dd irq_undefined ;reserved
		dd exc_coproc_error
