BITS 32

MULTIBOOT_MAGIC equ 0x1BADB002
MULTIBOOT_FLAGS equ 0 + 1<<16
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
MULTIBOOT_HEADER_PADDR equ multiBootHeader - 0xC0000000

LOWMEM_SZ:	equ 0x100000 ;1mb
PAGEDIRSIZE: equ 4096
PAGETABLESIZE: equ 4096


VMEM_OFFSET: equ 0xC0000000
RELOCTABLE_SIZE: equ 3*4
NROF_PAGEDIR_ENTRIES: equ 1024
NROF_PAGETABLE_ENTRIES: equ 1024
PAGEFLAGS: equ 0x03

extern kernelEnd
extern kmain
global __init:function
global bootInfo:data

SECTION multiboot
multiBootHeader:
dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM
dd MULTIBOOT_HEADER_PADDR
dd MULTIBOOT_HEADER_PADDR
dd 0
dd 0
dd (__init - 0xC0000000)

SECTION boottext

__init:
	;EXTREME BOOTSTRAPPING!!
	mov eax, 0xBEEFCAFE
	;je floppyBoot

	;setup gdt
	lgdt [(gdtrPhys - VMEM_OFFSET)]
	xchg bx, bx
	;mov eax, [edi]
	;sub eax, 0xC0000000
	;mov [edi], eax

	;reload segment registers
	;jmp $
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	;setup stack
	mov esp, stackEnd - VMEM_OFFSET
	mov ebp, esp

	;push 0x08
	;push .cont - VMEM_OFFSET
	;retf
	jmp 0x08:(.cont - VMEM_OFFSET)

	.cont:

	;setup paging
	push pageTable - VMEM_OFFSET
	push pageDir - VMEM_OFFSET
	sub ebp, 4

	;map page table
	xor ecx, ecx
	mov eax, PAGEFLAGS
	mov edi, [ebp]

	.start:
	cmp ecx, NROF_PAGETABLE_ENTRIES
	je .end
	stosd

	add eax, 0x1000
	inc ecx
	jmp .start

	.end:
	;map page dir
	;init everything to zero
	mov edi, [ebp-4]
	mov ecx, NROF_PAGEDIR_ENTRIES
	xor eax, eax

	rep stosd

	;add entry at 0x00000000
	mov edi, [ebp-4]
	mov eax, [ebp]
	or eax, PAGEFLAGS
	mov [edi], eax

	;and at 0xC0000000
	mov [edi+(0xC0000000 >> 20)], eax

	;also add entry at 0xFFC00000 to itself
	mov [edi+(1023*4)], edi

	mov eax, [ebp-4]
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	mov eax, 0xDEADBEEF
	push .cont2
	retn
	.cont2: ;we are now in paged mode so we can use virtual addresses safely
	lgdt [gdtr]
	xor eax, eax
	mov esp, stackEnd
	mov ebp, stackEnd
	mov [bootInfo], ebx

	;we can now get rid of the pde at 0x00000000
	mov [pageDir], eax

	call kmain

	;if it ever returns, halt
	cli
	.halt:
	hlt
	jmp .halt

relocTable:
dd stackEnd
dd pageDir
dd pageTable

SECTION .data
gdtr:	;dw (5*8) + (26 * 4)
		dw (gdtEnd - gdt)
		dd gdt

gdtrPhys:
		dw (gdtEnd - gdt)
		dd gdt - VMEM_OFFSET

gdt:
		dq 0 ;dummy
		;entry 0x08: kernel CS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 10011010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		;entry 0x10: kernel DS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 10010010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		;entry 0x18: usermode CS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 11111010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		;entry 0x20: usermode DS
		dw 0xFFFF	;limit 0:15
		dw 0x0000	;base 0:15
		db 0x00		;base 16:23
		db 11110010b	;access byte
		db 0xCF		;Flags = limit 16:19
		db 0x00		;base 24:31

		times 26 dd 0 ;room for tss
gdtEnd:

SECTION .bss align=4096
pageDir:
resb 0x1000
pageDirEnd:
pageTable:
resb 0x1000
pageTableEnd:
stackStart:
resb 0x4000
stackEnd:
bootInfo:
resd 1
