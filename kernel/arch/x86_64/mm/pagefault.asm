global excPF:function

extern puts
extern hexprintln
extern hexprintln64
extern cprint

extern panic
extern printk
extern puts
extern sysExit

extern mmGetEntry
extern allocPhysPage
extern allocCleanPhysPage
extern allocLargePhysPage
extern allocLargeCleanPhysPage
extern mmDoCOW
extern mmGetPageEntry

extern panicStack

%define endl 10, 0
%define NROF_PAGE_LEVELS 4

SECTION .text

invAlloc:
	mov rdi, invAllocMsg
	call puts
	jmp $

excPF:
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push rbx
	push r12

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x60], rax
	jae .noswapgs
		swapgs
		or [rsp + 0x80], dword 3
	.noswapgs:

    ;Check error code
    mov eax, [rsp + 0x58]
	cmp eax, 7
	je .writeFault

    test eax, 1
    jnz .error

	mov ebx, NROF_PAGE_LEVELS - 1
	.L1:
		mov rdi, cr2
		mov esi, ebx
		call mmGetEntry
		;rax contains pointer to pte
		mov r12, rax
		mov rax, [rax]
		test eax, 0x01 ;present flag
		jnz .L2
			test eax, (1 << 9) ;inuse flag
			jz .error ;give error if not in use
			cmp ebx, 0
			jne .L3
				test eax, (1 << 10) ;clean flag
				jz .L4
					call allocCleanPhysPage
					jmp .L7
				.L4:
					call allocPhysPage
					jmp .L7
			.L3:
			cmp ebx, 1
			jne invAlloc
				test eax, (1 << 10) ;clean flag
				jz .L6
					;call allocLargeCleanPhysPage
					jmp .L7
				.L6:
					call allocLargePhysPage
			.L7:
			mov rdx, 0xFFF0000000000FFF
			or rax, 1 ;set present bit
			and [r12], rdx  ;clear address field
			or [r12], rax ;OR it with new address + present bit
			jmp .return
		.L2:
		sub ebx, 1
		jns .L1
		;page already alloced
		mov rdi, weirdPF
		call puts
		jmp $

	.writeFault:
	mov rdi, cr2
	call mmDoCOW
	test eax, eax
	jnz .error

	.return:
	xor rdi, rdi
	mov cr2, rdi

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x60], rax
	jae .noswapgs2
		swapgs
	.noswapgs2:
	
	pop r12
	pop rbx
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	add rsp, 8 ;jump over error code
	iretq


    .error:
	add rsp, 0x58

	mov rdi, PFmsg2
	mov rsi, cr2
	mov rdx, [rsp + 8]
	mov rcx, [rsp]

	mov rax, 0xffffffff80000000
	cmp rdx, rax
	jae .panic
		call printk
		mov rdi, -1
		call sysExit
	.panic
	mov rax, [rsp + 0x20]
	mov [panicStack], rax
	call panic
	jmp $

SECTION .rodata
PFmsg2: db 'Page fault cr2:%X rip:%X error:%x', 10, 0

invAllocMsg:    db 'Invalid page alloc', endl

weirdPF:		db 'Weird Page fault occurred!', endl