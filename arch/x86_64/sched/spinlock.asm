global acquireSpinlock:function
global releaseSpinlock:function

extern sprint
extern hexprintln64

SECTION .text

acquireSpinlock:
	pushf
	cli

	xor ecx, ecx

	.lock:
	lock bts dword [rdi], 0
	jnc .noSpin
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
	test [rsp], dword (1 << 9)
	jz .noIF
		or [rdi], dword 2
		jmp .end
	.noIF:
		and [rdi], dword ~2
	.end:
	add rsp, 8
	ret

.error:
	mov rdi, spinlockStuckMsg
	call sprint
	mov rdi, [rsp + 8]
	call hexprintln64
	cli
	hlt

releaseSpinlock:
	xor eax, eax
	xchg [rdi], eax
	test eax, 2
	jz .noIF
		sti
	.noIF:
	ret

SECTION .rodata

spinlockStuckMsg: db 'Spinlock timed out.', 10, 'Ret addr: ', 0