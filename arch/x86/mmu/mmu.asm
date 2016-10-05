LOWMEM_SZ:	equ 0x100000 ;1mb

SECTION .text

global init_memory:function
init_memory:	;(void) returns void
		lgdt [gdtr] ;load new gdt
		xor eax, eax
		mov [ds:0xC0001000], eax ;clear first PDE
		mov eax, cr3
		mov cr3, eax
		ret

global TLB_update:function
TLB_update:	mov eax, cr3
		mov cr3, eax
		ret

SECTION .data

krnloff:	dd 0x00007000
stackoff:	dd 0x9c00

gdtr:		dw (5*8) + (26 * 4)
		dd gdt

gdt:
		dq 0 ;dummy
		;entry 0x08: kernel CS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 10011010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		;entry 0x10: kernel DS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 10010010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		;entry 0x18: usermode CS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 11111010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		;entry 0x20: usermode DS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 11110010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		times 26 dd 0 ;room for tss
