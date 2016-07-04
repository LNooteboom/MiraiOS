NROFIDTENTS:	equ 0x100
BYTESPERIDTENT:	equ 0x08
IDTOFFSET:	equ 0x3F00
NROFEXCINTS:	equ 17
NROFRESINTS:	equ 0x20
NROFPICINTS:	equ 0x10


SECTION .text

irq_PIT:	
		iret

irq_keyb:
		iret

irq_COM2:
		iret

irq_COM1:
		iret

irq_LPT2:
		iret

irq_floppy:
		iret

irq_LPT1_spurious:
		iret

irq_RTC:
		iret

irq_9:
		iret

irq_10:
		iret

irq_11:
		iret

irq_ps2mouse:
		iret

irq_coproc:
		iret

irq_ata1:
		iret

irq_ata2:
		iret

exc_diverror:	mov eax, 0xdeadbeef
		jmp $
		iret

exc_debug_error:
		iret

exc_breakpoint:
		iret

exc_overflow:
		iret

exc_bounds_check:
		iret

exc_inv_opcode:
		iret

exc_coproc_navail:
		iret

exc_double_fault: ;BSOD time!
		jmp $

exc_coproc_seg_overrun:
		iret

exc_invalid_tss:
		iret

exc_seg_npresent:
		iret

exc_stack:
		iret

exc_gen_prot:
		iret

exc_page_fault:
		iret

exc_coproc_error:
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

		;todo: add more idt entries

		lidt [idtr]
		mov edx, 0
		div edx ;deliberately cause crash
		jmp $

SECTION .data

idtr:		dw NROFIDTENTS * BYTESPERIDTENT
		dd IDTOFFSET

idt_undefined:	
		dw irq_undefined ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

idt_exception:
		dw exc_diverror ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_debug_error ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw irq_undefined ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_breakpoint ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_overflow ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_bounds_check ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_inv_opcode ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_coproc_navail ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_double_fault ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_coproc_seg_overrun ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_invalid_tss ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_seg_npresent ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_stack ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_gen_prot ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_page_fault ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw irq_undefined ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset

		dw exc_coproc_error ;lower word of offset
		dw 0x08 ;selector
		db 0 ;unused
		db 0x8E ;type + flags
		dw 0 ;high word of offset


