BITS 32

VMEM_OFFSET				equ 0xFFFFFFFF00000000
STACKSIZE				equ 0x4000

MULTIBOOT_MAGIC			equ 0x1BADB002
MULTIBOOT_FLAGS			equ (1 << 16) + (1 << 1)
MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
MULTIBOOT_HEADER_PADDR	equ multiBootHeader

PAGESIZE				equ 0x1000
LARGE_PAGE_SIZE_2			equ 0x200000
PAGEFLAGS				equ 0x03
PDEFLAGS				equ 0x83

global __init:function
global vmemOffset:data
global bootInfo:data

extern BSS_END_ADDR
extern TEXT_END_ADDR

SECTION multiboot
multiBootHeader:
	.magic:			dd MULTIBOOT_MAGIC
	.flags: 		dd MULTIBOOT_FLAGS
	.checksum:		dd MULTIBOOT_CHECKSUM
	.headerAddr:	dd MULTIBOOT_HEADER_PADDR
	.loadAddr:		dd MULTIBOOT_HEADER_PADDR
	.loadEndAddr:	dd 0 ;Load all
	.bssEnd:		dd (BSS_END_ADDR - VMEM_OFFSET)
	.entryAddr:		dd __init

SECTION boottext

__init:
	xchg bx, bx
	cmp eax, 0x2BADB002
	jne .noMultiBoot
		mov edx, 1
		jmp .L0
	.noMultiBoot:
		xor edx, edx
	
	.L0:
	mov [bootInfo], ebx
	mov ax, 0x10

	;setup temporary 32 bit gdt
	lgdt [gdtr32Phys]
	
	;load segment registers
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	;setup esp
	mov esp, stackStart
	mov ebp, stackStart

	jmp 0x08:(.cont)
	
	.cont:
	or edx, edx
	jnz .L1
		mov esi, noMultiBootMsg
		jmp bootError
	.L1:

	call detectCPUID

	call detectLongMode

	;long mode is detected, now we can setup paging
	;setup pml4t
	;add last entry pointing to itself
	;mov [PML4T + (511 * 8)], ((PML4T) + PAGEFLAGS)
	mov eax, [PML4TEntryToItself]
	mov edx, [PML4TEntryToItself + 4]
	mov [PML4T + (511 * 8)], eax
	mov [PML4T + (511 * 8) + 4], edx

	;add entry pointing to kernel 
	;mov [PML4T + (510 * 8)], ((PDPT) + PAGEFLAGS)
	mov eax, [PML4TEntryToKernel]
	mov edx, [PML4TEntryToKernel + 4]
	mov [PML4T + (510 * 8)], eax
	mov [PML4T + (510 * 8) + 4], edx

	;also add temporary entry at bottom to prevent page fault at switch
	;mov [PML4T], ((PDPT) + PAGEFLAGS)
	mov eax, [PML4TEntryTemp]
	mov edx, [PML4TEntryTemp + 4]
	mov [PML4T], eax
	mov [PML4T + 4], edx

	;add entry to PDPT
	;mov [PDPT], ((PDT) + PAGEFLAGS)
	mov eax, [PDPTEntry]
	mov edx, [PDPTEntry + 4]
	mov [PDPT], eax
	mov [PDPT + 4], edx

	;now fill PDT
	mov eax, PDEFLAGS ;entry low dword
	xor edx, edx ;entry high dword
	mov edi, PDT
	.L2:
		cmp eax, (BSS_END_ADDR - VMEM_OFFSET + PAGEFLAGS)
		jae .L3

		cmp eax, (TEXT_END_ADDR - VMEM_OFFSET + PAGEFLAGS)
		jb .L4
			or edx, 0x80000000 ;set NX bit
		.L4:
		mov [edi], eax
		mov [edi+4], edx
		add edi, 8

		add eax, LARGE_PAGE_SIZE_2

		dec ecx
		jnz .L2
	.L3:

	;now enable pae
	mov eax, cr4
	or eax, (1 << 5)
	mov cr4, eax

	;add pointer in cr3 to pml4t
	mov eax, PML4T
	mov cr3, eax


	;now enable longmode
	mov ecx, 0xC0000080 ;EFER register
	rdmsr
	or eax, (1 << 8) ;set LME bit
	wrmsr
	;enable paging to activate longmde
	mov eax, cr0
	or eax, (1 << 31)
	mov cr0, eax

	mov eax, 0xDEADBEEF
	hlt

detectLongMode:
	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001
	jb .noLongMode

	mov eax, 0x80000001
	cpuid
	test edx, 1 << 29
	jz .noLongMode

	nop
	ret

	.noLongMode:
		mov esi, noLongModeMsg
		jmp bootError

detectCPUID:
	;returns 0 if cpuid is not detected
	
	pushfd
	mov edx, [esp] ;copy flags in edx

	;flip ID bit
	xor [esp], dword (1 << 21)
	popfd

	pushfd
	pop eax

	;restore flags
	push edx
	popfd

	xor eax, edx ;returns zero if ID stays the same
	jnz .CPUIDPresent
		mov esi, noCPUIDMsg
		jmp bootError
	.CPUIDPresent:
	nop
	ret

bootError:
	;message in esi
	mov edi, 0xB8000

	.loop:
		lodsb
		or al, al
		jz .halt
		stosb
		mov al, 0x4F
		stosb
		jmp .loop
	
	.halt:
		hlt
		jmp .halt

SECTION bootdata

vmemOffset:
	dq VMEM_OFFSET

PML4TEntryToItself:
	dq (PML4T + PAGEFLAGS)
PML4TEntryToKernel:
	dq (PDPT + PAGEFLAGS)
PML4TEntryTemp:
	dq (PDPT + PAGEFLAGS)
PDPTEntry:
	dq (PDT + PAGEFLAGS)

gdtr32Phys:
	dw (gdt32End - gdt32)
	dd gdt32

gdt32:
	;entry 0x00: dummy
	dq 0
	;entry 0x08: text
	dw 0xFFFF	;limit	00:15
	dw 0x0000	;base	00:15
	db 0x00		;base	16:23
	db 0x9A		;Access byte: present, ring 0, executable, readable
	db 0xCF		;flags & limit 16:19: 4kb granularity, 32 bit
	db 0x00		;base	24:31
	;entry 0x10: data
	dw 0xFFFF	;limit	00:15
	dw 0x0000	;base	00:15
	db 0x00		;base	16:23
	db 0x92		;Access byte: present, ring 0, readable
	db 0xCF		;flags & limit 16:19: 4kb granularity, 32 bit
	db 0x00		;base	24:31
gdt32End:

noMultiBootMsg:
	db 'Invalid bootloader detected, please boot this operating system from a multiboot compliant bootloader (like GRUB)', 0

noCPUIDMsg:
	db 'No CPUID detected, please run this operating system on a 64 bit machine!', 0

noLongModeMsg:
	db 'This operating system requires a 64 bit CPU.', 0

SECTION bootbss align=4096 nobits
PML4T:
	resb PAGESIZE
PDPT:
	resb PAGESIZE
PDT:
	resb PAGESIZE
;Use PSE for kernel static memory
stackStart:
	resb STACKSIZE
stackEnd:
bootInfo:
	resq 1
