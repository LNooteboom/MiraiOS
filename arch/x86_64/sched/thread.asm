extern PAGE_SIZE
extern ackIRQ
extern initStackEnd
extern acquireSpinlock
extern releaseSpinlock
extern getCurrentThread
extern setCurrentThread
extern kthreadSwitch
extern kthreadFreeJoined
extern readyQueuePop
extern sleepSkipTime
extern deallocThread

global kthreadExit:function
global kthreadInit:function
global jiffyIrq:function
global migrateMainStack:function
global kthreadStop:function

SECTION .text

;thread stack layout:
;init | resume | register
;-08	 XX		return to exit
;Saved by interrupt handler
;-10	+98		ss
;-18	+90		rsp
;-20	+88		rflags
;-28	+80		cs
;-30	+78		rip
;Mandatory saved registers
;-38	+70		rax
;-40	+68		rcx
;-48	+60		rdx
;-50	+58		rdi
;-58	+50		rsi
;-60	+48		r8
;-68	+40		r9
;-70	+38		r10
;-78	+30		r11
;Optionally saved registers (only on task switch)
;-80	+28		rbx
;-88	+20		rbp
;-90	+18		r12
;-98	+10		r13
;-A0	+08		r14
;-A8	rsp		r15

kthreadInit:
	push rbp
	mov rbp, rsp
	
	mov [rdi - 0x30], rsi	;Point rip to function start
	mov [rdi - 0x50], rdx	;Put arg in rdi
	mov rax, kthreadReturn
	mov [rdi - 0x08], rax	;Put return address on stack
	mov rax, rdi
	sub rax, 0x08
	mov [rdi - 0x18], rax	;Set rsp
	sub rax, 0xA0
	mov [rdi], rax			;Set stack pointer in threadInfo

	mov eax, 0x10			;data segment
	mov edx, 0x08			;code segment
	mov [rdi - 0x10], rax	;Set ss
	mov [rdi - 0x28], rdx	;Set cs

	mov rsp, rdi
	sub rsp, 0x18
	pushfq					;Set rflags

	leave
	ret

kthreadReturn:
	mov rdi, rax
kthreadExit:
	mov r14, rdi ;r14 = thread return value
	call getCurrentThread
	mov r15, rax
	lea rdi, [rax + 0x14]
	call acquireSpinlock

	xor rdi, rdi
	call setCurrentThread

	mov [r15 + 8], r14 ;set return value
	mov [r15 + 16], dword 0 ;set threadstate to FINISHED

	mov eax, [r15 + 0x18]
	test eax, eax
	jnz nextThread ;don't free joined if thread is detached
		mov rdi, r15
		call kthreadFreeJoined
		jmp nextThread
	

jiffyIrq:
	;save mandatory registers
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

	call getCurrentThread
	push rax
	test rax, rax
	jz .noLock
	cmp [rax + 0x10], dword 1
	jne .noLock
		lea rdi, [rax + 0x14]
		call acquireSpinlock
	.noLock:
	mov rdi, [rsp]
	call sleepSkipTime
	mov esi, eax
	mov rdi, [rsp]
	test rdi, rdi
	jz .noThread
	call kthreadSwitch
	pop rdx

	cmp rax, rdx
	je .noSwitch
		test rdx, rdx
		jz .noSave ;skip if this cpu wasn't busy
			;task switch occured
			;save optional registers
			sub rsp, 0x30
			mov [rsp + 0x28], rbx
			mov [rsp + 0x20], rbp
			mov [rsp + 0x18], r12
			mov [rsp + 0x10], r13
			mov [rsp + 0x08], r14
			mov [rsp], r15
			;save rsp
			mov [rdx], rsp
		.noSave:
		;get new rsp
		mov rsp, [rax]
		;restore optional registers
		mov rbx, [rsp + 0x28]
		mov rbp, [rsp + 0x20]
		mov r12, [rsp + 0x18]
		mov r13, [rsp + 0x10]
		mov r14, [rsp + 0x08]
		mov r15, [rsp]
		add rsp, 0x30

		;release spinlock on new thread
		push rdx
		lea rdi, [rax + 0x14]
		call releaseSpinlock
		.noThread:
		pop rdx
	.noSwitch:
	test rdx, rdx
	jz .noReleaseOld
	cmp [rdx + 0x10], dword 1
	jne .noReleaseOld
		;release spinlock on old thread
		lea rdi, [rdx + 0x14]
		call releaseSpinlock
	.noReleaseOld:

	call ackIRQ
	;Restore mandatory registers
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

migrateMainStack:
	;rdi contains thread pointer
	mov r8, rdi

	std
	mov rsi, initStackEnd
	xor eax, eax
	;lea rcx, [initStackEnd - rsp]
	mov rcx, initStackEnd
	sub rcx, rsp
	add rcx, 8
	rep movsb
	cld

	;mov r8, rdi
	sub r8, initStackEnd
	add rsp, r8
	add rbp, r8
	ret

kthreadStop:
	;load return address in rdx
	pop rdx
	;setup stack for iret
	;push ss
	push 0x10
	;push stackpointer
	lea rax, [rsp + 8]
	push rax
	;push flags
	pushfq
	;push cs
	push 0x08
	;push return address
	push rdx

	
	;save opt regs only
	sub rsp, 0x78
	mov [rsp + 0x28], rbx
	mov [rsp + 0x20], rbp
	mov [rsp + 0x18], r12
	mov [rsp + 0x10], r13
	mov [rsp + 0x08], r14
	mov [rsp], r15

	call getCurrentThread

	test [rax + 0x14], dword 0x02 ;get IF on spinlock
	jz .noIRQ
		or [rsp + 0x88], dword (1 << 9) ;set IF on stack
	.noIRQ:

	mov [rax], rsp ;save rsp
	mov r15, rax

nextThread: ;r15 = old thread
	call readyQueuePop
	mov r14, rax ;r14 = new thread

	;Halt if no task is available
	test r14, r14
	jnz .load
		sti
		hlt
		cli
		jmp nextThread
	.load:

	mov rsp, [r14] ;switch to new stack

	mov rdi, r14
	call setCurrentThread

	mov r13d, [r15 + 0x18] ;get thread detached
	lea rdi, [r15 + 0x14]
	call releaseSpinlock ;release spinlock on old thread

	test r13d, r13d ;is the old thread detached?
	jz .notDetached
		;is the old thread finished?
		mov eax, [r15 + 0x10]
		test eax, eax
		jnz .notDetached
			;dealloc old thread
			mov rdi, r15
			call deallocThread
	.notDetached:

	;xchg bx, bx
	;Restore registers
	mov rax, [rsp + 0x70]
	mov rcx, [rsp + 0x68]
	mov rdx, [rsp + 0x60]
	mov rdi, [rsp + 0x58]
	mov rsi, [rsp + 0x50]
	mov r8,  [rsp + 0x48]
	mov r9,  [rsp + 0x40]
	mov r10, [rsp + 0x38]
	mov r11, [rsp + 0x30]
	mov rbx, [rsp + 0x28]
	mov rbp, [rsp + 0x20]
	mov r12, [rsp + 0x18]
	mov r13, [rsp + 0x10]
	mov r14, [rsp + 0x08]
	mov r15, [rsp]
	add rsp, 0x78
	iretq

;return:
;	ret