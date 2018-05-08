global setjmp:function
global longjmp:function
global _setjmp:function
global _longjmp:function

_setjmp:
setjmp: ;(jmp_buf buf)
	pop rcx
	;save callee saved regs
	mov [rdi], rcx ;ret addr
	mov [rdi + 0x08], rbx
	mov [rdi + 0x10], rbp
	mov [rdi + 0x18], rsp
	mov [rdi + 0x20], r12
	mov [rdi + 0x28], r13
	mov [rdi + 0x30], r14
	mov [rdi + 0x38], r15
	xor eax, eax
	jmp rcx ;return

_longjmp:
longjmp: ;(jmp_buf buf, int i)
	mov rax, rsi ;ret value
	mov rcx, [rdi] ;ret addr
	mov rbx, [rdi + 0x08]
	mov rbp, [rdi + 0x10]
	mov rsp, [rdi + 0x18]
	mov r12, [rdi + 0x20]
	mov r13, [rdi + 0x28]
	mov r14, [rdi + 0x30]
	mov r15, [rdi + 0x38]
	jmp rcx