%define IRQ_STUBS_START	32
%define IRQ_STUBS_END	128

global initIrqStubs:function
global syscallInit:function
;global registerSyscall:function
global initExecRetThread:function
global initForkRetThread:function
global sysSigRet:function ;should never be called from syscalltbl

extern syscallTable

extern mapIdtEntry

extern handleIRQ
extern ackIRQ

extern forkRet

extern sigRet
extern acquireSpinlock
extern releaseSpinlock
extern setCurrentThread
extern kfree
extern nextThread
extern sysExit

extern printk

SYSCALL_MAX:	equ 256

SECTION .text

initIrqStubs:
	push rbx
	mov ebx, 32
	.loop:
		lea rdi, [(irqStubsList - 32 * 8) + rbx * 8]
		mov esi, ebx
		xor edx, edx
		call mapIdtEntry
		inc ebx
		cmp ebx, 128
		jne .loop
	pop rbx
	ret

syscallInit:
	;set STAR
	mov ecx, 0xC0000081
	xor eax, eax
	mov edx, 0x0008 | (0x0018 << 16)
	wrmsr

	;set LSTAR
	mov rdx, syscallEntry64
	mov eax, edx
	mov ecx, 0xC0000082
	shr rdx, 32
	wrmsr

	;set SFMASK
	xor edx, edx
	mov eax, (1 << 9) ;set IF mask
	mov ecx, 0xC0000084
	wrmsr

	ret

initExecRetThread: ;(thread_t thread, void *start, uint64_t arg1, uint64_t arg2)
	mov rdi, [rdi]
	mov [rdi - 0x08], r8  ;rsp on sysret stack
	mov [rdi - 0x18], rsi ;rcx on sysret stack
	mov [rdi - 0x28], rcx ;rsi on sysret stack
	mov [rdi - 0x30], rdx ;rdi on sysret stack

	ret

initForkRetThread: ;(thread_t newThread, thread_t parent) returns void
	;sysret stack
	mov ecx, (0x88 / 8)
	sub rdi, 0x88
	sub rsi, 0x88
	rep movsq
	mov qword [rdi - 0x10], 0 ;set rax to 0

	;return addr
	mov rax, forkSysret64
	mov [rdi - 0x88], rax
	
	;iretq stack
	pushfq
	pop rdx
	or rdx, (1 << 9) ;set IF

	mov qword [rdi - 0x90], 0x10 ;ss
	lea rax, [rdi - 0x88]
	mov [rdi - 0x98], rax ;rsp
	mov [rdi - 0xA0], rdx ;rflags
	mov qword [rdi - 0xA8], 0x08 ;cs
	mov rax, forkRet
	mov [rdi - 0xB0], rax ;rip
	lea rax, [rdi - (0xB0 + 0x78)]
	mov [rdi], rax ;set stackpointer in thread struct

	ret

syscallEntry64:
	cmp eax, 0x15
	je sysSigRet
	swapgs
	mov [gs:0x28], rsp
	mov rsp, [gs:8]
	mov rsp, [rsp]

	push qword [gs:0x28]

	sti

	sub rsp, 0x50
	mov [rsp + 0x48], rax
	mov [rsp + 0x40], rcx
	mov [rsp + 0x38], rdx
	mov [rsp + 0x30], rsi
	mov [rsp + 0x28], rdi
	mov [rsp + 0x20], r8
	mov [rsp + 0x18], r9
	mov [rsp + 0x10], r10
	mov [rsp + 0x08], r11
	mov [rsp], rbx

	cld

	mov rcx, r10

	mov rbx, [rsp + 0x48]
	cmp rbx, SYSCALL_MAX
	mov eax, -7 ;-ENOSYS
	jae sysret64
	
	mov r10, [syscallTable + rbx * 8]
	test r10, r10
	jz sysret64 ;eax is still -ENOSYS

	cmp ebx, 7
	jne .noSaveForkRegs
		push rbp
		push r12
		push r13
		push r14
		push r15
	.noSaveForkRegs:

	call r10

	cmp ebx, 7
	jne sysret64
forkSysret64:
		pop r15
		pop r14
		pop r13
		pop r12
		pop rbp

sysret64:
	mov rdx, [gs:8]
	lea rcx, [rsp + 0x58]
	mov [rdx], rcx
	;rax need not be restored
	mov rcx, [rsp + 0x40]
	mov rdx, [rsp + 0x38]
	mov rsi, [rsp + 0x30]
	mov rdi, [rsp + 0x28]
	mov r8,  [rsp + 0x20]
	mov r9,  [rsp + 0x18]
	mov r10, [rsp + 0x10]
	mov r11, [rsp + 0x08]
	mov rbx, [rsp]

	cli
	mov rsp, [rsp + 0x50]
	
	swapgs
	db 0x48 ;no sysretq in NASM
	sysret

sysSigRet:
	;interrupts are disabled
	swapgs
	;don't bother saving regs
	mov rsp, [gs:0x10] ;switch to exception stack

	push rbx
	mov rbx, [gs:8]
	lea rdi, [rbx + 0x14]
	call acquireSpinlock

	cmp [rbx + 0x230], dword 0 ;sigDepth
	jne .valid
		;invalid signal return, exit
		mov edi, -1
		call sysExit
	.valid:
	sub [rbx + 0x230], dword 1

	push rax
	mov rdi, rsp
	call sigRet
	pop rsi ;struct SigRegs

	mov r8, rbx
	pop rbx

	mov rsp, [r8]

	mov rdx, [rsi + 0x78] ;rip
	mov rax, 0xffffffff80000000
	cmp [rsi + 0x78], rax
	jae .kernel
		push 0x23 ;ss
		push qword [rsi + 0x80] ;rsp
		push qword [rsi + 0x88] ;rflags
		push 0x2B ;cs
	.kernel:
		push 0x10
		push qword [rsi + 0x80]
		push qword [rsi + 0x88]
		push 0x08
	.end:
	push rdx ;rip
	mov r9, rsi

	;copy regs to stack
	cld
	sub rsp, 15*8
	mov ecx, 15
	mov rdi, rsp
	rep movsq

	cmp [r9 + 0x98], dword 2 ;state was either SCHEDWAIT or RUNNING
	jbe .return
	cmp [r8 + 0x234], dword 0 ;sigrun
	jne .return
		mov [r8 + 0x234], dword 0 ;set sigrun to 0
		mov rax, [r9 + 0x98] ;thread state
		mov [r8 + 0x10], rax

		mov qword [gs:8], 0 ;set current thread to NULL
		mov [r8], rsp

		;use exception stack
		mov rsp, [gs:0x10]

		push r9
		lea rdi, [r8 + 0x14]
		call releaseSpinlock
		pop rdi
		call kfree

		xor r15d, r15d
		jmp nextThread
	.return:

	push r9
	lea rdi, [r8 + 0x14]
	call releaseSpinlock
	pop rdi
	call kfree

	cli
	mov rax, 0xffffffff80000000
	cmp [rsp + 0x78], rax
	jae .noswapgs
		swapgs
	.noswapgs:
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	iretq

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
	mov [rsp + 0x28], rsi
	mov [rsp + 0x20], rdi
	mov [rsp + 0x18], r8
	mov [rsp + 0x10], r9
	mov [rsp + 0x08], r10
	mov [rsp], r11

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x50], rax
	jae .noswapgs
		swapgs
		or [rsp + 0x70], dword 3
	.noswapgs:

	mov rdi, [rsp + 0x48]
	call handleIRQ

	call ackIRQ

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x50], rax
	jae .noswapgs2
		swapgs
	.noswapgs2:

	;Restore registers
	mov rax, [rsp + 0x40]
	mov rcx, [rsp + 0x38]
	mov rdx, [rsp + 0x30]
	mov rsi, [rsp + 0x28]
	mov rdi, [rsp + 0x20]
	mov r8,  [rsp + 0x18]
	mov r9,  [rsp + 0x10]
	mov r10, [rsp + 0x08]
	mov r11, [rsp]
	add rsp, 0x50
	iretq

SECTION .rodata
teststr: db `%X %X\n`, 0