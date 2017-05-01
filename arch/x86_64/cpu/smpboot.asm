global smpbootStart:function

global smpboot16start:data
global smpboot16end:data

extern physLapicBase
extern lapicBase
extern cpuInfoSize
extern cpuInfos
extern nrofCPUs
extern initStackEnd
extern apExcStacks

SECTION .text

smpbootStart:
	xchg bx, bx
	;get APIC ID

	mov eax, [physLapicBase]
	mov edx, [physLapicBase + 4]
	mov ecx, 0x1B
	or eax, (1 << 11)
	wrmsr ;enable local apic

	mov rsi, [lapicBase]
	;get APIC ID
	mov edx, [rsi + 0x20]
	shr edx, 24

	;now get cpuinfo for this cpu
	mov rax, [cpuInfos]
	xor ecx, ecx
	mov rdi, [cpuInfoSize]
	.loop:
		cmp [rax + 8], edx ;compare this cpu's apic id
		je .end
		
		inc ecx
		add rax, rdi
		cmp ecx, [nrofCPUs]
		jne .loop
		nop
		jmp .error
	.end:
	;pointer to cpuinfo is in rax, index+1 in ecx
	dec ecx
	jnz .highExcStack
		mov rsp, initStackEnd
		mov rbp, initStackEnd
		jmp .end2
	.highExcStack:
		mov rsp, [apExcStacks]
		shl rcx, 12 ;Multiply index to page size
		add rsp, rcx
		mov rbp, rsp
	.end2:
	xchg bx, bx
	push 0
	jmp $

	.error:
	cli
	hlt
	jmp .error

SECTION .rodata

smpboot16start:
INCBIN "smpboot16.bin"
smpboot16end: