BITS 32

VMEM_OFFSET				equ 0xC0000000
STACKSIZE				equ 0x4000

MULTIBOOT_MAGIC			equ 0x1BADB002
MULTIBOOT_FLAGS			equ 0 + 1 << 16
MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
MULTIBOOT_HEADER_PADDR	equ multiBootHeader - VMEM_OFFSET

extern BSS_END_ADDR

global __init:function
global vmemOffset:data
global bootInfo:data

SECTION multiboot
multiBootHeader:
	.magic:			dd MULTIBOOT_MAGIC
	.flags: 		dd MULTIBOOT_FLAGS
	.checksum:		dd MULTIBOOT_CHECKSUM
	.headerAddr:	dd MULTIBOOT_HEADER_PADDR
	.loadAddr:		dd MULTIBOOT_HEADER_PADDR
	.loadEndAddr:	dd 0 ;Load all
	.bssEnd:		dd (BSS_END_ADDR - VMEM_OFFSET)
	.entryAddr:		dd (__init - VMEM_OFFSET)

SECTION .text

__init:
	cmp eax, 0x2BADB001
	jne .noMultiBoot
		mov edx, 1
		jmp .L0
	.noMultiBoot:
		xor edx, edx
	
	.L0:
	mov ax, 0x10
	xchg bx, bx

	;setup temporary 32 bit gdt
	lgdt [(gdtr32Phys - VMEM_OFFSET)]
	
	;load segment registers
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	;setup esp
	mov esp, (stackStart - VMEM_OFFSET)
	mov ebp, (stackStart - VMEM_OFFSET)

	jmp 0x08:(.cont - VMEM_OFFSET)
	
	.cont:
	or edx, edx
	jnz .L1
		mov esi, (noMultiBootMsg - VMEM_OFFSET)
		jmp bootError
	.L1:
	call detectCPUID
	hlt

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
		mov esi, (noCPUIDMsg - VMEM_OFFSET)
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
		mov al, 0xCF
		stosb
		jmp .loop
	
	.halt:
		hlt
		jmp .halt

SECTION .rodata

vmemOffset:
	dq VMEM_OFFSET

gdtr32Phys:
	dw (gdt32End - gdt32)
	dd (gdt32 - VMEM_OFFSET)

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

SECTION .data

SECTION .bss align=4096
stackStart:
	resb STACKSIZE
stackEnd:

bootInfo:
	resq 1
