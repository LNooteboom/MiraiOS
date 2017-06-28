global doPcpuRead64:function
global doPcpuWrite64:function
global doPcpuRead32:function
global doPcpuWrite32:function

extern hexprintln

SECTION .text

doPcpuRead64:
	mov rax, [gs:rdi]
	ret

doPcpuWrite64:
	mov [gs:rdi], rsi
	ret

doPcpuRead32:
	mov eax, [gs:rdi]
	ret

doPcpuWrite32:
	mov [gs:rdi], esi
	ret