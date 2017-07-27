BITS 64
DEFAULT REL

global bootInfo:data
global __init:function

global initStackEnd:data
global excStackStart:data

global nxEnabled:data

extern KERNEL_START_ADDR
extern TEXT_END_ADDR
extern DATA_END_ADDR
extern BSS_END_ADDR
extern VMEM_OFFSET

extern efiMain

%include "arch/x86_64/init/efi/efi.inc"

SECTION .setup align=16 exec

__init:
	push rcx
	mov r14, rdx ;r14 contains ptr to system table
	mov r15, [rdx + efiSysTable.bootServices] ;r15 contains ptr to bootservices table

	mov rax, 0x56524553544f4f42
	cmp rax, [r15] ;check boot services signature
	jne bootError

	;detect support for NX bit
	xor r13d, r13d
	mov eax, 0x80000001
	cpuid
	test edx, (1 << 20)
	jz .noNX
		mov r13d, 1 ;move 1 into r13 if NX is supported
	.noNX:

	;get total amount of pages to map
	call getNrofKernelPages
	mov rcx, rax
	shr rcx, 21 ;divide it by LARGEPAGE_SIZE to get nrof page tables required
	test rax, 0x1FFFFF
	jz .noAdd
		add rcx, 1
	.noAdd:
	mov rbx, rcx ;also store it in rbx for later use
	add rcx, 3 ;furthermore we need room for pml4t, pdpt and pdt

	;allocate pages
	sub rsp, 8 ;stack alignment
	mov rax, 0xFFFFF000 ;allocate below 4GB
	push rax
	mov rdi, [r15 + efiBootTable.allocatePages]
	mov esi, 1 ;alloc type = maxAddr
	mov edx, 2 ;memory type = loaderData
	;rcx still loaded with nrof pages
	mov r8, rsp ;where to put result
	call efiCall4
	test rax, rax
	jnz bootError
	pop rbp ;load result in rbp
	add rsp, 8

	;rbp + 0x0000 mmap buffer
	;rbp + 0x0000 kernel pml4t
	;rbp + 0x1000 kernel pdpt
	;rbp + 0x2000 kernel pdt
	;rbp + 0x3000+ kernel pts

	;zero all page tables except pml4t
	mov rdi, [r15 + efiBootTable.setMem]
	lea rdx, [rbx + 2] ;size
	lea rsi, [rbp + 0x1000] ;ptr to mem
	shl rdx, 12
	xor ecx, ecx
	call efiCall3

	;copy current pml4t to new pml4t
	mov rdi, [r15 + efiBootTable.copyMem]
	mov rsi, rbp ;destination = pml4t
	mov rdx, cr3 ;source = current pml4t
	mov ecx, 0x1000 ;size
	call efiCall3

	call preparePageTables
	
	;Set syscall extension enable + NX enable if it is supported
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 ;set SCE bit
	test r13d, r13d
	jz .noNX2
		or eax, (1 << 11) ;set NX bit enable
	.noNX2:
	wrmsr

	mov cr3, rbp

	mov rsi, r14
	pop rdi ;Get image handle
	;switch stack to initStack
	mov rsp, initStackEnd
	mov rax, efiMain
	jmp rax

preparePageTables: ;base in rbp, nrof page tables in rbx, mmap info in [rsp]
	;add top entry in pml4t
	;get physical pdpt addr
	lea rax, [rbp + 0x1003]
	mov [rbp + (511 * 8)], rax ;add pml4e

	;get physical pdt addr
	lea rdx, [rbp + 0x2003]
	mov [rbp + 0x1000 + (510 * 8)], rdx ;add pdpe

	;add pdes
	lea rax, [rbp + 0x3003]
	xor edx, edx
	.start:
		mov [rbp + 0x2040 + rdx*8], rax

		add edx, 1
		add rax, 0x1000
		cmp edx, ebx
		jne .start

	;add ptes
	lea rax, [rel __init]
	mov rdx, TEXT_END_ADDR
	and rax, ~0xFFF
	sub rdx, KERNEL_START_ADDR
	or rax, 1
	xor ecx, ecx
	lea rdi, [rbp + 0x3000]
	.start2:
		mov [rdi], rax

		add rcx, 0x1000
		add rdi, 8
		add rax, 0x1000
		cmp rcx, rdx
		jb .start2

	
	mov rdx, BSS_END_ADDR
	sub rdx, TEXT_END_ADDR
	test r13d, r13d
	jz .noNX
		mov r8, (1 << 63) ;nx bit
		or rax, r8
	.noNX:
	or rax, 3
	xor ecx, ecx
	.start3:
		mov [rdi], rax

		add rcx, 0x1000
		add rdi, 8
		add rax, 0x1000
		cmp rcx, rdx
		jb .start3
	ret

getNrofKernelPages: ;(void)
	mov rax, BSS_END_ADDR
	sub rax, KERNEL_START_ADDR
	test rax, 0xFFF
	jz .aligned
		and rax, ~0xFFF
		add rax, 0x1000
	.aligned:
	shr rax, 12
	ret

bootError:
	lea rdi, [rel errorMsg]
	call puts

	jmp $

puts:
	mov rdx, rdi
	mov rsi, [r14 + efiSysTable.conOut]
	mov rdi, [rsi + 8]
	jmp efiCall2

%if 0

hexprint:
	push rbx
	push r12

	mov r12, rdi
	mov ebx, 15*4
	.start:
		mov rdi, r12
		mov ecx, ebx

		shr rdi, cl
		and rdi, 0x0F
		cmp edi, 10
		jae .h
			add edi, __utf16__ '0'
			jmp .e
		.h:
			add edi, __utf16__ 'A' - 10
		.e:

		call putc
		sub ebx, 4
		jns .start

	pop r12
	pop rbx
	mov edi, __utf16__ `\n`
	call putc
	mov edi, __utf16__ `\r`
	;mov edi, __utf16__ `-`
	jmp putc

putc:
	and edi, 0xFFFF
	push rdi
	mov rdx, rsp
	mov rsi, [r14 + efiSysTable.conOut]
	mov rdi, [rsi + 8]
	call efiCall2
	add rsp, 8
	ret
%endif

efiCall2:
	sub rsp, 40
	mov rcx, rsi
	;mov rdx, rdx
	call rdi
	add rsp, 40
	ret

efiCall3:
	sub rsp, 40
	mov r8, rcx
	mov rcx, rsi
	;mov rdx, rdx
	call rdi
	add rsp, 40
	ret

efiCall4:
	sub rsp, 40
	mov r9, r8
	mov r8, rcx
	mov rcx, rsi
	;mov rdx, rdx
	call rdi
	add rsp, 40
	ret

errorMsg:
	db __utf16__ `Boot error occured\n\r\0`

SECTION .data

bootInfo: dq 0

nxEnabled: dd 0

SECTION .bss align=4096 nobits

excStackStart:
	resb 0x1000
initStackEnd: