global acquireSpinlock:function
global releaseSpinlock:function

extern sprint
extern hexprintln64

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
		cmp ecx, 1000000
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
	xchg bx, bx
	mov rdi, spinlockStuckMsg
	call sprint
	mov rdi, [rsp]
	call hexprintln64
	cli
	hlt

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