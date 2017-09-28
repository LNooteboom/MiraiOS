global initIDT:function
global mapIdtEntry:function
global unmapIdtEntry:function
global idtSetIST:function
global idtSetDPL:function

global idtr:data

NROF_IDT_ENTRIES	equ 0x100
BYTES_PER_IDT_ENTRY	equ 0x10

SECTION .text

initIDT:
	;(void) returns void
	;load idtr
	lidt[idtr]
	
	ret

mapIdtEntry:
	;(void (*ISR)(void), interrupt_t interrupt, uint8_t flags) returns void
	;flags bit 0 = interrupt clear, trap when set
	mov eax, (0x08 << 16)
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

unmapIdtEntry:
	;(interrupt_t interrupt) returns void
	shl rdi, 4
	and [idt + rdi + 4], word 0x7FFF ;clear present bit

	ret

idtSetIST: ;(interrupt_t interrupt, int ist)
	;mov [(idt + 4) + rdi * BYTES_PER_IDT_ENTRY], sil
	shl edi, 4
	mov [idt + rdi + 4], sil
	ret

idtSetDPL: ;(interrupt_t interrupt, bool user)
	test esi, esi
	;lea rdi, [(idt + 4) + rdi * BYTES_PER_IDT_ENTRY]
	shl edi, 4
	add rdi, idt + 4
	jnz .user
		and [rdi], dword ~(3 << 13)
		ret
	.user:
	or [rdi], dword (3 << 13)
	ret

SECTION .rodata

idtr:
	dw (NROF_IDT_ENTRIES * BYTES_PER_IDT_ENTRY)
	dq idt

SECTION .bss

idt:
	resb (NROF_IDT_ENTRIES * BYTES_PER_IDT_ENTRY)
