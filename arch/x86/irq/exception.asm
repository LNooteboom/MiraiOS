extern routeInterrupt

extern sprint

global initExceptions:function

SECTION .text

initExceptions:
		push ebp
		mov ebp, esp

		push 1
		push 0
		push excDivError
		call routeInterrupt
		add esp, 0xC

		leave
		ret

excDivError:	;mov eax, 0xdeadbeef
		push ebp
		mov ebp, esp
		push diverrormsg
		call sprint
		add esp, 4
		jmp $
		;iret

exc_debug_error:
exc_breakpoint:
exc_overflow:
exc_bounds_check:
exc_inv_opcode:
exc_coproc_navail:
exc_double_fault: ;BSOD time!
exc_coproc_seg_overrun:
exc_invalid_tss:
exc_seg_npresent:
exc_stack:
exc_gen_prot:
exc_page_fault:
exc_coproc_error:
		jmp $
		iret

SECTION .rodata

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

