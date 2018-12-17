NROF_DEFINED_EXCS: equ 21

extern panic
extern printk
extern mapIdtEntry
extern lapicBase

extern tssSetIST
extern idtSetIST
extern idtSetDPL

extern excPF

extern kthreadExit
extern excExit

global initExceptions:function
global undefinedInterrupt:function
global dummyInterrupt:function

SECTION .text

initExceptions:
    push rbx
    push r12

    xor ebx, ebx
    mov r12, excList
    
    .start:
        cmp ebx, NROF_DEFINED_EXCS
        jae .end
        xor edx, edx
        mov esi, ebx
        mov rdi, [r12]
        call mapIdtEntry
        add r12, 8
        inc ebx
        jmp .start
    .end:

	mov rdi, 3
	mov esi, 1
	call idtSetDPL

    pop r12
	pop rbx
    ret

excBaseErrorCode:
	xchg bx, bx
	push rax ;rsp+8 = irq num, +16 = error
	mov rax, [rsp + 8]
	xchg [rsp + 16], rax ;error becomes irq num
	xchg [rsp], rax ;store error in temp, load old rax
	mov [rsp + 8], rax ;store old rax
	pop rax ;load error in rax
	jmp __excBase
excBase:
	xchg bx, bx
	push rax
	xor eax, eax
__excBase: ;error code in rax
	sub rsp, 0x40
	mov [rsp + 0x38], rcx
	mov [rsp + 0x30], rdx
	mov [rsp + 0x28], rdi
	mov [rsp + 0x20], rsi
	mov [rsp + 0x18], r8
	mov [rsp + 0x10], r9
	mov [rsp + 0x08], r10
	mov [rsp], r11

	test [rsp + 0x54], dword 0x80000000
	jnz .noswapgs
		swapgs
		or [rsp + 0x68], dword 3
	.noswapgs:

	mov r10, [rsp + 0x48]
	mov rdi, excMessageBase
	mov rsi, [excMsgList + r10 * 8]
	mov rdx, rax
	mov rcx, [rsp + 0x50]
	call printk

	cmp [rsp + 0x48], dword 3
	je .return

	;sti
	xchg bx, bx
	mov rax, [gs:8]
	test rax, rax
	jz .kThread
	cmp [rax + 0x20], dword 0
	jne .userThread
		.kThread:
		;call kthreadExit
		call panic
		jmp .halt
	.userThread:
		mov rdi, [rsp + 0x48]
		mov rsi, [rsp + 0x50]
		call excExit
    .halt:
	cli
	hlt
	jmp .halt

	.return:
	test [rsp + 0x54], dword 0x80000000
	jnz .noswapgs2
		swapgs
	.noswapgs2:

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

excDE:
	push 0
	jmp excBase

excDB:
	push 1
	jmp excBase

excNMI:
	iretq

excBP:
	push 3
	jmp excBase

excOF:
	push 4
	jmp excBase

excBR:
	push 5
	jmp excBase

excUD:
	push 6
	jmp excBase

excNM:
	;push 7
	;jmp excBase
	;Enable SSE for this thread
	or [rsp + 0x20], dword 3
	push rax
	swapgs

	clts

	mov rax, [gs:8]
	cmp [rax + 0x28], dword 0
	jne .rstor
		;initialize sse regs
		mov [rax + 0x28], dword 1 ;set thread->floatsUsed
		ldmxcsr [defaultMxcsr]
		movaps xmm0, [sseZero]
		movaps xmm1, [sseZero]
		movaps xmm2, [sseZero]
		movaps xmm3, [sseZero]
		movaps xmm4, [sseZero]
		movaps xmm5, [sseZero]
		movaps xmm6, [sseZero]
		movaps xmm7, [sseZero]
		movaps xmm8, [sseZero]
		movaps xmm9, [sseZero]
		movaps xmm10, [sseZero]
		movaps xmm11, [sseZero]
		movaps xmm12, [sseZero]
		movaps xmm13, [sseZero]
		movaps xmm14, [sseZero]
		movaps xmm15, [sseZero]
		jmp .out
	.rstor:
		fxrstor [rax + 0x30]
	.out:
	swapgs
	pop rax
	iretq

excDF:
	;push 8
	;jmp excBaseErrorCode
	xchg bx, bx
	mov rdi, DFmsg
	call panic

excCSO:
	push 9
	jmp excBase

excTS:
	push 10
	jmp excBaseErrorCode

excNP:
	push 11
	jmp excBaseErrorCode

excSS:
	push 12
	jmp excBaseErrorCode

excGP:
	push 13
	jmp excBaseErrorCode

excMF:
	push 16
	jmp excBase

excAC:
	push 17
	jmp excBaseErrorCode

excMC:
	push 18
	jmp excBase

excXM:
	;iretq
	push 19
	jmp excBase

excVE:
	push 20
	jmp excBase

ALIGN 8
undefinedInterrupt:
    iretq

ALIGN 8
dummyInterrupt:
	push rax
	mov rax, [lapicBase]
	mov [rax + 0xB0], dword 0
	pop rax
	iretq

SECTION .rodata

ALIGN 16
sseZero: 	dq 0
			dq 0
defaultMxcsr: dd 0x01100

excMessageBase:	db 'CPU exception: %s', 10, 'Error code: %d', 10, 'At: %x', 10, 0
nullStr: db 0

ALIGN 8
excList:
dq excDE
dq excDB
dq excNMI
dq excBP
dq excOF
dq excBR
dq excUD
dq excNM
dq excDF
dq excCSO
dq excTS
dq excNP
dq excSS
dq excGP
dq excPF
dq undefinedInterrupt
dq excMF
dq excAC
dq excMC
dq excXM
dq excVE

excMsgList:
dq DEmsg
dq nullStr
dq DBmsg
dq BPmsg
dq OFmsg
dq BRmsg
dq UDmsg
dq NMmsg
dq DFmsg
dq CSOmsg
dq TSmsg
dq NPmsg
dq SSmsg
dq GPmsg
dq MFmsg
dq ACmsg
dq XMmsg
dq VEmsg

DEmsg:	db 'Division error', 0
DBmsg:	db 'Debug error', 0
BPmsg:	db 'Breakpoint reached', 0
OFmsg:	db 'Overflow', 0
BRmsg:	db 'BOUND Range exceeded', 0
UDmsg:	db 'Invalid opcode detected', 0
NMmsg:	db 'Coprocessor not available', 0
DFmsg:	db 'Double Fault', 0
CSOmsg:	db 'Coprocessor segment overrun', 0
TSmsg:	db 'Invalid TSS', 0
NPmsg:	db 'Segment not present', 0
SSmsg:	db 'Stack fault', 0
GPmsg:	db 'General protection fault', 0
MFmsg:	db 'Coprocessor error', 0
ACmsg:	db 'Alignment check', 0
MCmsg:	db 'Machine check', 0
XMmsg:	db 'SIMD floating point error', 0
VEmsg:	db 'Virtualization exception', 0