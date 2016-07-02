NROFIDTENTS:	equ 0x100
BYTESPERIDTENT:	equ 0x08
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

irq_reserved:	iret

irq_undefined:	iret

SECTION .data

idtr:		dw NROFIDTENTS * BYTESPERIDTENT
		dd idt

idt:		;reserved interrupts
		;resb NROFRESINTS * BYTESPERIDTENT
		;start of maskable interrupts + PIC
		
