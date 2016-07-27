BITS 32

NROFIDTENTS:	equ 0x100
BYTESPERIDTENT:	equ 0x08
IDTOFFSET:	equ 0x3800
NROFEXCINTS:	equ 17
NROFPICINTS:	equ 0x10

extern errorscreen
extern sprint
extern currentattrib
extern pic_eoi
extern cursorX
extern cursorY
extern hexprint

extern presskey

SECTION .text

irq_PIT:	xor eax, eax
		mov [cursorX], eax
		mov [cursorY], eax
		mov al, [currentattrib]
		push eax
		mov eax, [counter]
		inc dword [counter]
		push eax
		call hexprint
		add esp, 8
		push 0
		call pic_eoi
		add esp, 4
		iret

irq_keyb:	in al, 0x60
		push eax
		call presskey
		add esp, 4
		push 1
		call pic_eoi
		add esp, 4
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
		mov eax, [ss:ebp+4] ;get old eip
		push eax ;and push it
		mov eax, diverrormsg
		push eax
		call errorscreen
		jmp $
		;iret

exc_debug_error:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, dbgerror
		push eax
		call errorscreen
		jmp $
		iret

exc_breakpoint:
		push ebp
		mov ebp, esp
		mov al, [currentattrib]
		push eax
		mov eax, breakpointerr
		push eax
		call sprint
		leave
		iret

exc_overflow:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, overflowerr
		push eax
		call errorscreen
		jmp $

exc_bounds_check:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, bndrangeerr
		push eax
		call errorscreen
		jmp $

exc_inv_opcode:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, invopcodeerr
		push eax
		call errorscreen
		jmp $
		iret

exc_coproc_navail:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, coprocnavail
		push eax
		call errorscreen
		jmp $
		iret

exc_double_fault: ;BSOD time!
		push ebp
		mov esp, ebp
		mov eax, [ss:ebp+8]
		push eax
		mov eax, dblfault
		push eax
		call errorscreen
		
		jmp $

exc_coproc_seg_overrun:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, coprocsegoverrun
		push eax
		call errorscreen
		jmp $
		iret

exc_invalid_tss:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, invalidTSSerr
		push eax
		call errorscreen
		jmp $
		iret

exc_seg_npresent:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, segnpresent
		push eax
		call errorscreen
		jmp $
		iret

exc_stack:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, stackfault
		push eax
		call errorscreen
		jmp $
		iret

exc_gen_prot:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, genprot
		push eax
		call errorscreen
		jmp $
		iret

exc_page_fault:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, pagefault
		push eax
		call errorscreen
		jmp $
		iret

exc_coproc_error:
		push ebp
		mov ebp, esp
		mov eax, [ss:ebp+4]
		push eax
		mov eax, coprocerr
		push eax
		call errorscreen
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

		mov ax, es
		mov [ss:ebp-4], ax
		mov ax, 0x10
		mov es, ax
		xor cx, cx
		mov edi, IDTOFFSET
	.start:	cmp cx, NROFIDTENTS
		je .cont
		mov eax, [idt_undefined]
		stosd
		mov eax, [idt_undefined + 4]
		stosd

		inc cx
		jmp .start

	.cont:	;we now have a template idt
		;now fill it for the important interrupts
		;exceptions first
		mov ecx, NROFEXCINTS * BYTESPERIDTENT
		mov edi, IDTOFFSET
		mov esi, idt_exception
		rep movsb

		;now the pic interrupts
		mov ecx, NROFPICINTS * BYTESPERIDTENT
		mov edi, IDTOFFSET + (0x20 * BYTESPERIDTENT)
		mov esi, idt_pic
		rep movsb

		;and finally the main kernel interrupt
		mov ecx, BYTESPERIDTENT
		mov edi, IDTOFFSET + (0x80 * BYTESPERIDTENT)
		mov esi, idt_krnl
		rep movsb

		lidt [idtr]
		mov edx, 0
		;div edx ;deliberately cause crash
		;sti
		;int 0x21 ;fake keyboard interrupt
		mov ax, [ss:bp-4]
		mov es, ax
		leave
		ret

global crashtest:function
crashtest:	;jmp $
		mov ebx, 0
		mov eax, 1
		mov edx, 0
		div ebx
		jmp $

SECTION .data

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

idt_undefined:	
	;	dw irq_undefined ;lower word of offset
	;	dw 0x08 ;selector
	;	db 0 ;unused
	;	db 0x8F ;type + flags
	;	dw 0 ;high word of offset

idt_krnl:
	;	dw kernel_int ;lower word of offset
	;	dw 0x08 ;selector
	;	db 0 ;unused
	;	db 0x8F ;type + flags
	;;	dw 0 ;high word of offset

idt_pic:
	;	dw irq_PIT ;lower word of offset
	;	dw 0x08 ;selector
	;	db 0 ;unused
	;	db 0x8E ;type + flags
	;	dw 0 ;high word of offset


idt_exception:
	;	dw exc_diverror ;lower word of offset
	;	dw 0x08 ;selector
	;	db 0 ;unused
	;	db 0x8F ;type + flags
	;	dw 0 ;high word of offset
