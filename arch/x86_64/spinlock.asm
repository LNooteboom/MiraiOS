BITS 32

global acquireSpinlock:function
global releaseSpinlock:function

extern irqEnabled

acquireSpinlock: ;(spinlock_t *lock) returns void
	mov edx, [esp + 4]
	xor eax, eax
	mov cl, 1

	.spin:
	cli
	lock cmpxchg [edx], cl
	jz .end
	sti
	pause
	jmp .spin

	.end:
	ret

releaseSpinlock: ;(spinlock_t *lock) returns void
	mov eax, [esp + 4]
	mov [eax], byte 0
	cmp [irqEnabled], byte 0
	je .cont
	sti
	.cont:
	ret
