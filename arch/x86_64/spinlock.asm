global acquireSpinlock:function
global releaseSpinlock:function

extern irqEnabled

acquireSpinlock: ;(spinlock_t *lock) returns void
	xor eax, eax
	mov cl, 1

	cli
	.spin:
		lock cmpxchg [rdi], cl
		jz .end
		pause
		jmp .spin

	.end:
	ret

releaseSpinlock: ;(spinlock_t *lock) returns void
	mov [rdi], byte 0
	cmp [irqEnabled], byte 0
	je .cont
		sti

	.cont:
	nop
	ret

