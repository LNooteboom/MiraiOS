BITS 32

VMEM_OFFSET				equ 0xC0000000

MULTIBOOT_MAGIC			equ 0x1BADB002
MULTIBOOT_FLAGS			equ 0 + 1 << 16
MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
MULTIBOOT_HEADER_PADDR	equ multiBootHeader - VMEM_OFFSET

extern BSS_END_ADDR

global __init:function
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
	mov eax, 0xBEEFCAFE
	hlt

bootInfo:
	dd 0
