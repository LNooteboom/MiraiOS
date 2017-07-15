global init64:function

global bootInfo:data

extern initStackEnd
extern gdtr
extern PML4T
extern PDPT
extern kmain
extern __stack_chk_guard
extern VMEM_OFFSET

STRUC BootInfo
.mmap: resq 1
.mmapLen: resq 1
.cmdline: resq 1
.lowMemReservedEnd: resq 1
.initrd: resq 1
.initrdLen: resq 1
.size:
ENDSTRUC

STRUC Mmap
.addr: resq 1
.nrofPages: resq 1
.attr: resq 1
.size:
ENDSTRUC

SECTION .text
init64: ;We are now in 64-bit
	;jump to correct high address
	mov rax, .cont
	jmp rax

	.cont:
	;ok now reset stack
	lea rsp, [initStackEnd]
	lea rbp, [initStackEnd]

	;reload gdtr
	lgdt [gdtr]

	;generate stack guard with tsc
	rdtsc
	shl rdx, 32
	or rax, rdx
	neg rax
	mov [__stack_chk_guard], rax

	;prepare bootInfo
	;multibootInfo ptr in edi
	mov esi, edi
	add rsi, VMEM_OFFSET

	;translate mmap
	mov edx, [rsi + 48]
	mov ecx, [rsi + 44]
	mov rax, VMEM_OFFSET + 0x500
	add rdx, VMEM_OFFSET

	mov [bootInfo + BootInfo.mmapLen], rcx ;store len
	mov [bootInfo + BootInfo.mmap], rax ;store addr

	test ecx, ecx
	jz .halt
	xor r8d, r8d
	.start:
		mov r10, [rdx + 12] ;size
		mov r9, [rdx + 4] ;addr
		mov r11d, [rdx + 20] ;type

		shr r10, 12 ;convert size to nrofpages
		test r9, r9
		jnz .noAdd
			;ignore BDA + bootInfo
			sub r10, 1
			add r9, 0x1000
		.noAdd:

		;save everything in new mmap
		mov [rax + Mmap.addr], r9
		mov [rax + Mmap.nrofPages], r10
		mov [rax + Mmap.attr], r11

		add rax, Mmap.size
		sub ecx, 0x18
		add rdx, 0x18
		test ecx, ecx
		jnz .start

	;prepare the rest of bootInfo
	;copy cmdline to [rax]
	mov r15, rsi
	mov rdi, rax
	cld

	test [rsi], dword (1 << 2)
	jz .noCmdLine
	mov esi, [rsi + 16]
	test rsi, rsi
	jz .noCmdLine
		add rsi, VMEM_OFFSET
		;store new ptr to cmdline
		mov [bootInfo + BootInfo.cmdline], rdi
		.start2:
		lodsb
		test al, al
		stosb
		jnz .start2
		jmp .end
		
	.noCmdLine:
		mov [bootInfo + BootInfo.cmdline], rsi
	.end:
	;set lowmemReservedEnd
	mov [bootInfo + BootInfo.lowMemReservedEnd], rdi

	;set initrd
	;check if exists
	cmp [r15], dword (1 << 3)
	jz .noInitrd
	cmp [r15 + 20], dword 1
	jb .noInitrd
		mov rax, [r15 + 24]
		mov edx, [rax]
		mov ecx, [rax + 4]
		sub ecx, edx
		mov [bootInfo + BootInfo.initrd], rdx
		mov [bootInfo + BootInfo.initrdLen], rcx
		jmp .end2
	.noInitrd:
		xor eax, eax
		mov [bootInfo + BootInfo.initrd], rax
		mov [bootInfo + BootInfo.initrdLen], rax
	.end2:
	call kmain
	;kmain should never return
	.halt:
		cli
		hlt
		jmp .halt

SECTION .bss
bootInfo: resb BootInfo.size