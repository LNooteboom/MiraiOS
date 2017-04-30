global pcpuRead:function
global pcpuWrite:function

SECTION .text

pcpuRead:
	mov rax, [gs:rdi]
	ret

pcpuWrite:
	mov rax, rsi
	mov [gs:rdi], rax
	ret
