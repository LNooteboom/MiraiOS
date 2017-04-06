global init64:function

extern initStackEnd
extern gdtr
extern PML4T
extern PDPT
extern kmain
extern __stack_chk_guard

SECTION .text
init64: ;We are now in 64-bit
	;jump to correct high address
	lea rax, [ABS .cont]
	jmp rax

	.cont:
	;ok now reset stack
	lea rsp, [initStackEnd]
	lea rbp, [initStackEnd]

	;reload gdtr
	lgdt [gdtr]

	;remove temp PML4T and PDPT entries
	xor rax, rax
	mov [PML4T], rax
	mov [PDPT], rax

	;generate stack guard with tsc
	rdtsc
	shl rdx, 32
	or rax, rdx
	neg rax
	mov [__stack_chk_guard], rax

	call kmain

	.halt:
		cli
		hlt
		jmp .halt
