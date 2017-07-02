SMPCALL_IRQ_NUM:	equ 0xC1

global smpCallInit:function
global smpCallFunction:function

extern lapicSendIPIToAll
extern acquireSpinlock
extern releaseSpinlock
extern nrofActiveCPUs
extern routeInterrupt
extern ackIRQ

SECTION .text

smpCallInit:
	mov rdi, smpCallIrq
	mov esi, SMPCALL_IRQ_NUM
	xor edx, edx
	jmp routeInterrupt ;call + ret

ALIGN 8
smpCallFunction: ;(void (*func)(void* arg), void *arg, bool wait) returns void
	push rbx
	push r12
	push r13
	mov rbx, rdi
	mov r12, rsi
	mov r13d, edx

	cmp [nrofActiveCPUs], dword 1
	je .exit

	.wait:
		pause
		cmp [smpCallCpuCount], dword 0
		jne .wait

	mov rdi, smpCallLock
	call acquireSpinlock

	mov [smpCallFunc], rbx
	mov [smpCallArg], r12
	mov [smpCallCpuCount], dword 1

	;now send IPI
	mov edi, SMPCALL_IRQ_NUM
	xor esi, esi
	call lapicSendIPIToAll

	test r13d, r13d
	jz .noWait
		mov eax, [nrofActiveCPUs]
		.wait2:
			pause
			cmp [smpCallCpuCount], eax
			jne .wait2
	.noWait:

	mov rdi, smpCallLock
	call releaseSpinlock

	.exit:
	pop r13
	pop r12
	pop rbx
	ret

ALIGN 8
smpCallIrq:
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

	mov rax, [smpCallFunc]
	mov rdi, [smpCallArg]
	call rax

	;lock inc dword [smpCallCpuCount]
	mov eax, 1
	lock xadd [smpCallCpuCount], eax
	inc eax
	cmp eax, [nrofActiveCPUs]
	jne .exit
		mov [smpCallCpuCount], dword 0
	.exit:

	call ackIRQ
	mov rax, [rsp + 0x40]
	mov rcx, [rsp + 0x38]
	mov rdx, [rsp + 0x30]
	mov rdi, [rsp + 0x28]
	mov rsi, [rsp + 0x20]
	mov r8,  [rsp + 0x18]
	mov r9,  [rsp + 0x10]
	mov r10, [rsp + 0x08]
	mov r11, [rsp]
	add rsp, 0x48
	iretq

SECTION .bss

ALIGNB 8
smpCallFunc:		resq 1
smpCallArg:			resq 1
smpCallCpuCount:	resd 1
smpCallLock:		resd 1
