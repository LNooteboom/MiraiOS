BITS 64

global acquireSpinlock:function
global releaseSpinlock:function

extern irqEnabled

acquireSpinlock: ;(spinlock_t *lock) returns void
	xor eax, eax
	mov cl, 1

	.spin:
		cli
		lock cmpxchg [rdi], cl
		jz .end
		sti
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
	ret

