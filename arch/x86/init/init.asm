LOWMEM_SZ:	equ 0x100000 ;1mb
PAGEFLAGS: equ 0x03
PAGEDIRSIZE: equ 4096
PAGETABLESIZE: equ 4096

extern kernelEnd
extern kmain

SECTION multiboot

SECTION boottext

bootstrap:
	;mov [multiBootInfo], ebx

	;setup paging
	;initialise page directory to 0
	mov edi, pageDir
	mov ecx, PAGEDIRSIZE
	mov al, 0x00
	rep stosb

	;initialise page table to 0
	mov edi, pageTable
	mov ecx, PAGETABLESIZE
	rep stosb

	;identity map the whole page
	mov eax, PAGEFLAGS
	mov edi, pageTable
	xor ecx, ecx
	.start:
	cmp ecx, PAGETABLESIZE / 4
	jae .end
	stosd
	add eax, 0x1000
	inc ecx
	jmp start

	.end:
	;add entry in page directoty pointing to page table
	mov eax, pageTable
	or eax, PAGEFLAGS
	mov edi, pageDir
	add edi, (0xC0000000 >> 22)
	mov [edi], eax
	;also add same entry to pos 0x00000000 to prevent page fault
	;(This entry will be removed later)
	mov [pageDir], eax

	;add entry pointing to itself
	mov eax, pageDir
	or eax, PAGEFLAGS
	mov edi, pageDirEnd
	sub edi, 0x04
	mov [edi], eax

	lgdt [gdtr]
	jmp 0x08:dword .next

.next:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	mov esp, stackEnd
	mov ebp, esp

	;We can now use the stack

	call kmain

	;if kmain ever returns:
	cli
	
	.hang:
	hlt
	jmp .hang


global init_memory:function
init_memory:	;(void) returns void
		lgdt [gdtr] ;load new gdt
		xor eax, eax
		mov [ds:0xC0001000], eax ;clear first PDE
		mov eax, cr3
		mov cr3, eax
		ret

global TLB_update:function
TLB_update:	mov eax, cr3
		mov cr3, eax
		ret

SECTION bootdata
gdtr:	;dw (5*8) + (26 * 4)
		dw (gdtEnd - gdt)
		dd gdt

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

[SECTION bootbss nobits align=0x1000]
pageDir:
resb 0x1000
pageDirEnd:
pageTable:
resb 0x1000
pageTableEnd:
stackStart:
resb 0x4000
stackEnd:

