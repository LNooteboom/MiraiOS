
BITS 16

	nop
	nop
	nop
	nop
	mov ax, 0x7000
	mov ds, ax
	mov es, ax
	mov dx, 0xBEEF
	xchg bx, bx
	jmp $