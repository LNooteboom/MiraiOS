BITS 32

NROFIDTENTS:	equ 0x100
BYTESPERIDTENT:	equ 0x08
IDTOFFSET:	equ 0xC0006000
NROFEXCINTS:	equ 17
NROFPICINTS:	equ 0x10

extern panic
extern sprint
extern pic_eoi
extern cursorX
extern cursorY
extern hexprint
extern newline

extern presskey

SECTION .text

irq_PIT:	xor eax, eax
		mov [cursorX], eax
		mov [cursorY], eax
		mov eax, [counter]
		inc dword [counter]
		push eax
		call hexprint
		add esp, 8
		push 0
		call pic_eoi
		add esp, 4
		iret

irq_keyb:	push edx
		push ecx
		push eax
		in al, 0x60
		push eax
		call presskey
		add esp, 4
		push 1
		call pic_eoi
		add esp, 4
		pop eax
		pop ecx
		pop edx
		iret

irq_COM2:	push 3
		call pic_eoi
		add esp, 4
		iret

irq_COM1:	push 4
		call pic_eoi
		add esp, 4
		iret

irq_LPT2:	push 5
		call pic_eoi
		add esp, 4
		iret

irq_floppy:	push 6
		call pic_eoi
		add esp, 4
		iret

irq_LPT1_spurious:push 7
		call pic_eoi
		add esp, 4
		iret

irq_RTC:	push 8
		call pic_eoi
		add esp, 4
		iret

irq_9:		push 9
		call pic_eoi
		add esp, 4
		iret

irq_10:		push 10
		call pic_eoi
		add esp, 4
		iret

irq_11:		push 11
		call pic_eoi
		add esp, 4
		iret

irq_ps2mouse:	push 12
		call pic_eoi
		add esp, 4
		iret

irq_coproc:	push 13
		call pic_eoi
		add esp, 4
		iret

irq_ata1:	push 14
		call pic_eoi
		add esp, 4
		iret

irq_ata2:	push 15
		call pic_eoi
		add esp, 4
		iret

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
		call hexprint
		add esp, 4
		call newline
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

kernel_int:
		iret

irq_undefined:	iret

global irq_init:function
irq_init:	;(void) returns void
		push ebp
		mov ebp, esp
		sub esp, 4
		push edi
		push esi

		cld
		mov ax, es
		mov [ss:ebp-4], ax
		mov ax, 0x10
		mov es, ax
		xor cx, cx
		mov edi, IDTOFFSET

	.start:	cmp cx, NROFIDTENTS
		je .cont
		mov eax, irq_undefined
		stosw
		mov ax, 0x08
		stosw
		mov al, 0x00
		stosb
		mov al, 0x8F
		stosb
		mov eax, irq_undefined
		shr eax, 16
		stosw

		inc cx
		jmp .start

	.cont:	;we now have a template idt
		;now fill it for the important interrupts
		;exceptions first
		mov ax, ds
		mov es, ax
		xor cl, cl
		mov edi, IDTOFFSET ;interrupt table is from 0x6000 - 0x9FFF
		mov esi, exc_list
	.start2:cmp cl, NROFEXCINTS
		jge .end2
		movsw ;low word of offset
		mov ax, 0x08 ;Selector in GDT
		stosw
		mov al, 0x00 ;Zero
		stosb
		mov al, 0x8F ;type and flags
		stosb
		movsw ;high word of offset
		inc cl
		jmp .start2

	.end2:	;now irqs:
		xor cl, cl
		mov esi, irq_list
		mov edi, (32 * 8) + IDTOFFSET
	.start3:cmp cl, NROFPICINTS
		jge .end3
		movsw
		mov ax, 0x08 ;selector in GDT
		stosw
		mov al, 0x00
		stosb
		mov al, 0x8E ;type
		stosb
		movsw
		inc cl
		jmp .start3

	.end3:	;now the kernel interrupt
		mov edx, kernel_int
		mov edi, (0x80 * 8) + IDTOFFSET
		mov ax, dx
		stosw
		mov ax, 0x08
		stosw
		mov al, 0x00
		stosb
		mov al, 0x8F
		stosb
		shr edx, 16
		mov ax, dx
		stosw


		lidt [idtr]
		mov edx, 0
		;div edx ;deliberately cause crash
		;sti
		;int 0x21 ;fake keyboard interrupt
		mov ax, [ss:ebp-4]
		;mov es, ax
		pop edi
		pop esi
		leave
		ret

global crashtest:function
crashtest:	;jmp $
		mov ecx, 0
		mov eax, 1
		mov edx, 0
		div ecx
		jmp $

SECTION .data

regs:		db 'eax: ', 0
		db 'ebx: ', 0
		db 'ecx: ', 0
		db 'edx: ', 0
regsend:

counter:	dd 0
keybmsg:	db 'Keyboard interrupt received.', 0

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

idtr:		dw NROFIDTENTS * BYTESPERIDTENT
		dd IDTOFFSET

exc_list:
		dd exc_diverror
		dd exc_debug_error
		dd irq_undefined ;NMI
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
		dd irq_undefined ;reserved
		dd exc_coproc_error

irq_list:
		dd irq_PIT
		dd irq_keyb
		dd irq_COM2
		dd irq_COM1
		dd irq_LPT2
		dd irq_floppy
		dd irq_LPT1_spurious
		dd irq_RTC
		dd irq_9
		dd irq_10
		dd irq_11
		dd irq_ps2mouse
		dd irq_coproc
		dd irq_ata1
		dd irq_ata2
