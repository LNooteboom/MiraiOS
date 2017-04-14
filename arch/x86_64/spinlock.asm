global acquireSpinlock:function
global releaseSpinlock:function

SECTION .text

spinlockSpin:
	pause
	test [rdi], dword 1
	jnz spinlockSpin

acquireSpinlock:
	pushf
	cli

	.lock:
	lock bts dword [rdi], 0
	jnc .noSpin
	.spin:
		pause
		test [rdi], dword 1
		jnz .spin
		jmp .lock

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

releaseSpinlock:
	xor eax, eax
	xchg [rdi], eax
	test eax, 2
	jz .noIF
		sti
	.noIF:
	ret
