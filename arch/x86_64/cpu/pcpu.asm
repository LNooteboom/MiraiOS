global pcpuRead:function
global pcpuWrite:function

global smpboot16start:data
global smpboot16end:data

SECTION .text

pcpuRead:
	mov rax, [gs:rdi]
	ret

pcpuWrite:
	mov rax, rsi
	mov [gs:rdi], rax
	ret

SECTION .rodata

smpboot16start:
INCBIN "smpboot16.bin"
smpboot16end: