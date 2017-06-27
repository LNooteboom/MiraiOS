	.file	"jiffy.c"
	.intel_syntax noprefix
	.comm	jiffyCounter,8,8
	.comm	jiffyTimer,8,8
	.comm	jiffyVec,4,4
	.text
	.globl	jiffyInit
	.type	jiffyInit, @function
jiffyInit:
.LFB0:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	edi, 32
	call	kmalloc@PLT
	mov	QWORD PTR jiffyTimer[rip], rax
	mov	rax, QWORD PTR jiffyTimer[rip]
	mov	rdi, rax
	call	i8253Init@PLT
	mov	rax, QWORD PTR jiffyTimer[rip]
	movzx	eax, BYTE PTR 24[rax]
	movzx	eax, al
	mov	edx, 1
	mov	rcx, QWORD PTR jiffyIrq@GOTPCREL[rip]
	mov	rsi, rcx
	mov	edi, eax
	call	routeHWIRQ@PLT
	mov	DWORD PTR jiffyVec[rip], eax
	mov	edx, 0
	mov	esi, 193
	mov	rax, QWORD PTR reschedIPI@GOTPCREL[rip]
	mov	rdi, rax
	call	routeInterrupt@PLT
	mov	rax, QWORD PTR jiffyTimer[rip]
	mov	rax, QWORD PTR [rax]
	mov	edi, 1000
	call	rax
	mov	rax, QWORD PTR jiffyTimer[rip]
	mov	rax, QWORD PTR 8[rax]
	mov	edi, 1
	call	rax
	mov	eax, 0
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	jiffyInit, .-jiffyInit
	.globl	jiffyFini
	.type	jiffyFini, @function
jiffyFini:
.LFB1:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	rax, QWORD PTR jiffyTimer[rip]
	mov	rdi, rax
	call	kfree@PLT
	nop
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	jiffyFini, .-jiffyFini
	.ident	"GCC: (Ubuntu 6.3.0-12ubuntu2) 6.3.0 20170406"
	.section	.note.GNU-stack,"",@progbits
