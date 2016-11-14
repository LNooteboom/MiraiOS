BITS 32

global acquireSpinlock:function
global releaseSpinlock:function

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
	sti
	ret
