%define IRQ_STUBS_START	32
%define IRQ_STUBS_END	128

global initIrqStubs:function

extern mapIdtEntry

extern handleIRQ
extern ackIRQ

SECTION .text

initIrqStubs:
	push rbx
	mov rbx, 0
	.loop:
		lea rdi, [irqStubsList + rbx * 8]
		lea rsi, [rbx + 32]
		xor edx, edx
		call mapIdtEntry
		inc rbx
		cmp rbx, (128 - 32)
		jne .loop
	pop rbx
	ret

ALIGN 8
irqStubsList:
	%assign i IRQ_STUBS_START
	%rep IRQ_STUBS_END - IRQ_STUBS_START
		push i
		jmp irqCommon
		ALIGN 8
		%assign i i+1
	%endrep

irqCommon:
	;save registers
	sub rsp, 0x48
	mov [rsp + 0x40], rax
	mov [rsp + 0x38], rcx
	mov [rsp + 0x30], rdx
	mov [rsp + 0x28], rdi
	mov [rsp + 0x20], rsi
	mov [rsp + 0x18], r8
	mov [rsp + 0x10], r9
	mov [rsp + 0x08], r10
	mov [rsp], r11

	mov rdi, [rsp + 0x48]
	call handleIRQ

	call ackIRQ

	;Restore registers
	mov rax, [rsp + 0x40]
	mov rcx, [rsp + 0x38]
	mov rdx, [rsp + 0x30]
	mov rdi, [rsp + 0x28]
	mov rsi, [rsp + 0x20]
	mov r8,  [rsp + 0x18]
	mov r9,  [rsp + 0x10]
	mov r10, [rsp + 0x08]
	mov r11, [rsp]
	add rsp, 0x50
	iretq