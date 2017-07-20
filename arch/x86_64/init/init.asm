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

;framebuffer info
.fbAddr: resq 1
.fbSize: resq 1
.fbXRes: resd 1
.fbYRes: resd 1
.fbPitch: resd 1

.fbIsRgb: resd 1
.fbBpp: resd 1
.fbRSize: resb 1
.fbRShift: resb 1
.fbGSize: resb 1
.fbGShift: resb 1
.fbBSize: resb 1
.fbBShift: resb 1
.fbResSize: resb 1
.fbResShift: resb 1

.size:
ENDSTRUC

STRUC Mmap
.addr: resq 1
.nrofPages: resq 1
.attr: resq 1
.size:
ENDSTRUC

STRUC VbeInfo
.attr: resw 1
.winA: resb 1
.winB: resb 1
.gran: resw 1
.winSize: resw 1
.segA: resw 1
.segB: resw 1
.winFunc: resd 1
.pitch: resw 1

.xRes: resw 1
.yRes: resw 1
.xChar: resb 1
.yChar: resb 1
.nrofPlanes: resb 1
.bpp: resb 1
.nrofBanks: resb 1
.memModel: resb 1
.bankSize: resb 1
.nrofImagePages: resb 1
.reserved: resb 1

.rMaskSize: resb 1
.rShift: resb 1
.gMaskSize: resb 1
.gShift: resb 1
.bMaskSize: resb 1
.bShift: resb 1
.resMaskSize: resb 1
.resShift: resb 1
.dcAttr: resb 1

.physBase: resd 1
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
	;set video info
	cmp [r15], dword (1 << 11)
	jz .noFB
	mov ebp, [r15 + 76] ;get vbe mode info
	add rbp, VMEM_OFFSET

	;compute fb size
	movzx eax, word [rbp + VbeInfo.pitch]
	movzx edx, word [rbp + VbeInfo.yRes]
	mul edx
	mov [bootInfo + BootInfo.fbSize], eax

	;get other attributes
	mov eax, [rbp + VbeInfo.physBase]
	movzx ebx, word [rbp + VbeInfo.xRes]
	movzx ecx, word [rbp + VbeInfo.yRes]
	movzx edx, word [rbp + VbeInfo.pitch]
	mov rsi, [rbp + VbeInfo.rMaskSize] ;get all masks and shifts in one fell swoop
	movzx edi, byte [rbp + VbeInfo.bpp]
	mov r8b, [rbp + VbeInfo.memModel]

	;check if framebuffer valid
	cmp r8b, 6
	je .valid
	cmp r8b, 3
	jne .noFB
	cmp edi, 32
	jne .noFB
	jmp .rgb
	.valid: ;if (memModel == 6 || (memModel == 3 && bpp == 32))
		;check if rgb
		mov r9, 0x1808000808081008
		cmp rsi, r9
		je .rgb
		xor r9d, r9d
		jmp .end3
		.rgb:
		mov r9d, dword 1
	.end3:
	;store everything
	mov [bootInfo + BootInfo.fbAddr], rax
	mov [bootInfo + BootInfo.fbXRes], ebx
	mov [bootInfo + BootInfo.fbYRes], ecx
	mov [bootInfo + BootInfo.fbPitch], edx
	mov [bootInfo + BootInfo.fbIsRgb], r9d
	mov [bootInfo + BootInfo.fbBpp], edi
	mov [bootInfo + BootInfo.fbRSize], rsi
	jmp .end4
	.noFB:
	xor rax, rax
	mov [bootInfo + BootInfo.fbAddr], rax
	.end4:

	call kmain
	;kmain should never return
	.halt:
		cli
		hlt
		jmp .halt

SECTION .bss
bootInfo: resb BootInfo.size