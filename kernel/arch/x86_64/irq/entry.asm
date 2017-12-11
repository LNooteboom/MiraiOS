%define IRQ_STUBS_START	32
%define IRQ_STUBS_END	128

global initIrqStubs:function
global syscallInit:function
global registerSyscall:function
global initExecRetThread:function
global initForkRetThread:function

extern mapIdtEntry

extern handleIRQ
extern ackIRQ

extern forkRet

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
	;mov r8, rdi
	;mov r9, rcx
	;mov r10, rsi
	;sub rdi, 0x48 ;don't overwrite flags (r11)
	;mov ecx, (0x48 / 8)
	;xor eax, eax
	;rep stosq

	;mov [r8 - 0x18], r10 ;ret addr
	;mov [r8 - 0x28], r9
	;mov [r8 - 0x30], rdx

	mov [rdi - 0x18], rsi
	mov [rdi - 0x28], r9
	mov [rdi - 0x30], rdx

	ret

initForkRetThread: ;(thread_t newThread, thread_t parent) returns void
	mov rax, [rsi - 0x08]
	mov [rdi - 0x08], rax ;copy userspace stack pointer

	mov rax, [rsi - 0x18]
	mov [rdi - 0x18], rax ;copy ret addr (rcx)

	mov rax, [rsi - 0x50]
	;and rax, ~(1 << 9) ;delet this
	mov [rdi - 0x50], rax ;copy flags (r11)
	
	;rdi - 0x10 -> -0x58 userspace regs

	;return addr
	mov rax, sysret64
	mov [rdi - 0x58], rax
	
	;iretq stack
	pushfq
	mov qword [rdi - 0x60], 0x10 ;ss
	lea rax, [rdi - 0x58]
	mov [rdi - 0x68], rax ;rsp
	pop qword [rdi - 0x70] ;rflags
	mov qword [rdi - 0x78], 0x08 ;cs
	mov rax, forkRet
	mov [rdi - 0x80], rax ;rip
	lea rax, [rdi - 0xF8]
	mov [rdi], rax ;set stackpointer in thread struct

	ret

syscallEntry64:
	swapgs
	mov [gs:0x28], rsp
	mov rsp, [gs:8]

	push qword [gs:0x28]

	sti

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

	mov rcx, r10

	mov r10d, [rsp + 0x40]
	cmp r10d, SYSCALL_MAX
	mov eax, -7 ;-ENOSYS
	jae sysret64
	
	mov r10, [syscallTable + r10 * 8]
	test r10, r10
	jz sysret64 ;eax is still -ENOSYS

	call r10

sysret64:

	mov rdi, [rsp + 0x38]
	
	;rax need not be restored
	mov rcx, [rsp + 0x38]
	mov rdx, [rsp + 0x30]
	mov rsi, [rsp + 0x28]
	mov rdi, [rsp + 0x20]
	mov r8,  [rsp + 0x18]
	mov r9,  [rsp + 0x10]
	mov r10, [rsp + 0x08]
	mov r11, [rsp]

	mov rsp, [rsp + 0x48]
	
	swapgs
	db 0x48 ;no sysretq in NASM
	sysret

registerSyscall:
	mov [syscallTable + rdi * 8], rsi
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

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x50], rax
	jae .noswapgs
		swapgs
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
	mov rdi, [rsp + 0x28]
	mov rsi, [rsp + 0x20]
	mov r8,  [rsp + 0x18]
	mov r9,  [rsp + 0x10]
	mov r10, [rsp + 0x08]
	mov r11, [rsp]
	add rsp, 0x50
	iretq

SECTION .bss

syscallTable:
resq SYSCALL_MAX