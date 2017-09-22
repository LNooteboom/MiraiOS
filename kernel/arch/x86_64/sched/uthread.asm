
;userspace thread stack layout:
;init | resume | register
;Saved by interrupt handler
;-08	+98		ss
;-10	+90		rsp
;-18	+88		rflags
;-20	+80		cs
;-28	+78		rip
;Mandatory saved registers
;-30	+70		rax
;-38	+68		rcx
;-40	+60		rdx
;-48	+58		rdi
;-50	+50		rsi
;-58	+48		r8
;-60	+40		r9
;-68	+38		r10
;-70	+30		r11
;Optionally saved registers (only on task switch)
;-78	+28		rbx
;-80	+20		rbp
;-88	+18		r12
;-90	+10		r13
;-98	+08		r14
;-A0	rsp		r15

uthreadInit: ;(struct ThreadInfo *info (rdi), func *start(rsi), uint64_t arg1 (rdx), uint64_t arg2 (rcx), uint64_t userspaceStackpointer (r8))
	pushfq
	pop rax
	
	mov [rdi - 0x08], qword 0x10 ;ss
	mov [rdi - 0x10], r8 ;rsp
	mov [rdi - 0x18], rax ;rflags
	mov [rdi - 0x20], qword 0x18 ;cs = 64-bit usermode text
	mov [rdi - 0x28], rsi ;rip

	mov [rdi - 0x48], rdx
	mov [rdi - 0x50], rcx

	;set stackpointer in ThreadInfo to kernel stack
	lea rax, [rdi - 0x98]
	mov [rdi], rax

	ret