
BITS 16

	jmp short start

infoStart:
nxEnabled:		dw 0
contAddr:		dd 0
pml4tAddr:		dd 0

start:
	cli
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax

	mov eax, 0xA0
	mov cr4, eax ;enable pae and pge

	mov eax, [pml4tAddr]
	mov cr3, eax ;put pml4t addr in cr3

	mov ecx, 0xC0000080
	rdmsr
	or eax, (1 << 8) ;set LME bit
	cmp [nxEnabled], word 0
	je .noNX
		or eax, (1 << 11) ;set NX bit
	.noNX:
	wrmsr

	lidt [idtr]

	mov eax, cr0
	or eax, (1 << 0) | (1 << 31) ;enable paging and pmode
	mov cr0, eax

	xor eax, eax
	mov ax, cs
	shl eax, 4
	add eax, gdt
	mov [gdtr + 2], eax

	lgdt [gdtr]

	;prepare jumpvect
	mov eax, [contAddr]
	mov [jumpVect], eax

	db 0x66
	jmp far [jumpVect]

ALIGN 4
jumpVect:
	dd 0
	dw 0x08

idtr:
	dw 0
	dd 0

gdtr:
	dw (gdtEnd - gdt)
	dd 0

gdt:
	;entry 0x00: dummy
	dq 0
	;entry 0x08: 64 bit kernel text
	dw 0xFFFF	;limit	00:15
	dw 0x0000	;base	00:15
	db 0x00		;base	16:23
	db 0x98		;Access byte: present, ring 0
	db 0x2F		;flags & limit 16:19: 64-bit
	db 0x00		;base	24:31
	;entry 0x10: data
	dw 0xFFFF	;limit	00:15
	dw 0x0000	;base	00:15
	db 0x00		;base	16:23
	db 0x92		;Access byte: present, ring 0, readable
	db 0xCF		;flags & limit 16:19: 4kb granularity, 32 bit
	db 0x00		;base	24:31
gdtEnd: