TLB_INVAL_IRQ_NUM:	equ 0xC0

global tlbInit:function
global tlbInvalidateGlobal:function
global tlbInvalidateLocal:function
global tlbReloadCR3:function
global tlbReloadCR3Local:function

extern mapIdtEntry
extern acquireSpinlock
extern releaseSpinlock
extern ackIRQ
extern PAGE_SIZE
extern nrofActiveCPUs
extern lapicSendIPIToAll

SECTION .text

tlbInit:
	mov rdi, tlbInvalIrq
	mov esi, TLB_INVAL_IRQ_NUM
	xor edx, edx
	call mapIdtEntry
	ret

tlbInvalidateGlobal: ;(void *base, uint64_t numPages) returns void
	push rbx
	push r12
	mov rbx, rdi
	mov r12, rsi

	call tlbInvalidateLocal

	mov rdi, tlbInvalLock
	call acquireSpinlock

	mov [tlbInvalCPUCount], dword 1

	mov [tlbInvalNumPages], r12
	mov [tlbInvalStart], rbx

	;Send tlb ipi
	mov edi, TLB_INVAL_IRQ_NUM
	xor esi, esi
	call lapicSendIPIToAll

	;wait until all CPUs finished
	%if 0
	mov eax, [nrofActiveCPUs]
	popfq
	.wait:
		pause
		cmp [tlbInvalCPUCount], eax
		jne .wait
	
	%endif
	mov rdi, tlbInvalLock
	call releaseSpinlock

	pop r12
	pop rbx
	ret

tlbReloadCR3Local:
	mov rax, cr3
	mov cr3, rax
	ret

tlbReloadCR3:
	mov rax, cr3
	mov cr3, rax

	mov rdi, tlbInvalLock
	call acquireSpinlock

	mov [tlbDoReloadCR3], dword 1

	;Send tlb ipi
	mov edi, TLB_INVAL_IRQ_NUM
	xor esi, esi
	call lapicSendIPIToAll

	;wait until all CPUs finished
	%if 0
	mov eax, [nrofActiveCPUs]
	.wait:
		pause
		cmp [tlbInvalCPUCount], eax
		jne .wait
	%endif
	mov rdi, tlbInvalLock
	call releaseSpinlock
	sti
	ret

msg: db 'inval', 0
msg2: db 'e',10,0

tlbInvalIrq:
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

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x48], rax
	jae .noswapgs
		swapgs
		or [rsp + 0x68], dword 3
	.noswapgs:

	;mov rdi, msg
	;call printk

	cmp [tlbDoReloadCR3], dword 0
	jne .reloadCR3

	mov rsi, [tlbInvalNumPages]
	mov rdi, [tlbInvalStart]
	call tlbInvalidateLocal

	.exit:
	;;mov rdi, msg2
	;call printk
	lock inc dword [tlbInvalCPUCount]
	call ackIRQ

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x48], rax
	jae .noswapgs2
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
	add rsp, 0x48
	iretq

	.reloadCR3:
	mov rax, cr3
	mov cr3, rax
	jmp .exit

tlbInvalidateLocal: ;(void *base, uint64_t numPages)
	inc rsi
	jmp .chk
	.start:
		invlpg [rdi]
		add rdi, PAGE_SIZE
		.chk:
		dec rsi
		jne .start
	repz ret

SECTION .bss

ALIGNB 4
tlbDoReloadCR3:		resd 1

tlbInvalLock:		resd 1
tlbInvalCPUCount:	resd 1

ALIGNB 8
tlbInvalStart:		resq 1
tlbInvalNumPages:	resq 1
