BITS 64

DEFAULT REL

global initIDT:function
global routeInterrupt:function
global unrouteInterrupt:function

NROF_IDT_ENTRIES	equ 0x100
BYTES_PER_IDT_ENTRY	equ 0x10

SECTION .text

initIDT:
	;(void) returns void
	;load idtr
	lidt[idtr]
	
	ret

routeInterrupt:
	;(void (*ISR)(void), interrupt_t interrupt, uint8_t flags) returns void
	;flags bit 0 = interrupt clear, trap when set
	mov eax, (0x18 << 16)
	shl rsi, 4
	mov ecx, edi

	test dl, 1
	jnz .L0
		;interrupt gate
		mov cx, 0x8E00
		jmp .L1
	.L0:
		;trap gate
		mov cx, 0x8F00
	.L1:

	mov ax, di
	shr rdi, 32
	mov [idt + rsi], eax
	mov [idt + rsi + 4], ecx
	mov [idt + rsi + 8], edi
	mov [idt + rsi + 12], dword 0

	ret

unrouteInterrupt:
	;(interrupt_t interrupt) returns void
	shl rdi, 4
	and [idt + rdi + 4], word 0x7FFF ;clear present bit

	ret

SECTION .rodata

idtr:
	dw (NROF_IDT_ENTRIES * BYTES_PER_IDT_ENTRY)
	dq idt

SECTION .bss

idt:
	resb (NROF_IDT_ENTRIES * BYTES_PER_IDT_ENTRY)
