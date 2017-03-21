
global jiffyIrq:function

extern jiffyCounter
extern ackIRQ
extern decprint
extern sprint

SECTION .text

jiffyIrq:
	push rax
	inc qword [jiffyCounter]
	mov rax, [jiffyCounter]
	cmp rax, [destCount]
	jl .end
	add rax, 0x20
	mov [destCount], rax

	mov rdi, timerMsg
	call sprint
	mov rdi, [jiffyCounter]
	shr rdi, 5
	call decprint

	.end:
	call ackIRQ
	pop rax
	iretq

SECTION .data

timerMsg: db 0x0D, 'counter: ', 0
;timerMsg: db 'c: ', 0

SECTION .bss

destCount: resq 0