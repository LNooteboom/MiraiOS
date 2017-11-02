global acquireSpinlock:function
global releaseSpinlock:function

extern puts
extern hexprintln64

extern panic

SECTION .text

acquireSpinlock:
	pushf

	xor ecx, ecx
	pop rdx

	.lock:
	cli
	lock bts dword [rdi], 0
	jnc .noSpin
		test edx, (1 << 9)
		jz .spin
		sti

		.spin:
		inc ecx
		pause
		test [rdi], dword 1
		jz .lock
		;cmp ecx, 0x7FFFFFFF ;10000000000
		cmp ecx, 10000000
		jae .error
		jmp .spin

	.noSpin:

	;We now have the lock, read IF from stack
	test edx, (1 << 9)
	jz .noIF
		or [rdi], dword 2
		jmp .end
	.noIF:
		and [rdi], dword ~2
	.end:
	ret

.error:
	;mov eax, 0xDEADBEEF
	;mov rbx, [rsp]
	;mov rcx, [rsp + 0x08]
	;mov rdx, [rsp + 0x10]
	;mov rsi, [rsp + 0x18]
	;mov rdi, [rsp + 0x20]
	;mov r8, [rsp + 0x28]
	;mov r9, [rsp + 0x30]
	;jmp $
	xchg bx, bx
	mov rdi, spinlockStuckMsg
	;call puts
	;mov rdi, [rsp]
	;call hexprintln64
	;cli
	;hlt
	call panic

releaseSpinlock:
	xor eax, eax
	mfence
	xchg [rdi], eax
	test eax, 2
	jz .noIF
		sti
	.noIF:
	ret

SECTION .rodata

spinlockStuckMsg: db 'Spinlock timed out.', 10, 'Ret addr: ', 0