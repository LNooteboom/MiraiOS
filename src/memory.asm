
SECTION .text

global init_memory:function
init_memory:	;(void) returns void
		push ebp
		mov ebp, esp
		sub esp, 0x0C

		mov ax, es
		mov [ss:bp-4], ax
		pushf ;store flags

		lgdt [gdtr]

		mov ax, 0x0010 ;full data descriptor
		mov es, ax

		;now setup paging
		mov ecx, 1024 ;amount of entries: 1024
		mov eax, 0x00000002 ;write allowed, not present
		mov edi, 0x1000
		cld
		rep stosd

		;add entry that points to the (future) kernel page table (at 0x2000)
		mov eax, 0x00002000 ;address
		mov al, 0x09 ;present + supervisor only + write allow
		mov ebx, 0x1000
		mov [es:0x1000], eax ;and store it
		;jmp $

		;now fill kernel page table ...
		mov ecx, 1024
		mov eax, 0x00000000 ;nothing allowed
		mov edi, 0x2000
		cld
		rep stosd

		;...and setup the used pages properly
		mov eax, [krnloff]
		mov [ss:bp-0xC], eax
		xor ecx, ecx
		mov ebx, 0x2000

	.start:	cmp ecx, [krnl_memsz]
		jge .cont
		;add to page table
		mov eax, [ss:bp-0xC]
		mov al, 0x07
		mov [es:bx], eax

		add ecx, 0x1000 ;4kb
		add ebx, 0x04 ;bytes per entry
		;jmp .start

		;replace this:
		mov eax, 0x7007
		mov [es:(0x2000 + (7*4))], eax

	.cont:	
		mov eax, 0x1000
		mov cr3, eax

		;reload all segment registers
		mov ax, 0x0010
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		;jmp $

	.next:	;now enable paging
		mov eax, cr0
		or eax, 0x80000000
		mov cr0, eax
		;jmp $
		jmp 8:.n

	.n:	mov eax, 0x12345678
		mov esp, ebp
		pop ebp
		ret

SECTION .data

krnloff:	dd 0x00007000

krnl_memsz:	dd 0x1000

gdtr:		dw gdt_end-gdt-1
gdtr_offset:	dd gdt + 0x7000

gdt:		;first, a null descriptor 0x00
		dq 0
		;code descriptor 0x08, base 0, limit 0xffffffff, type 0x9A
		dw 0xFFFF ;limit 0:15
		dw 0x0000 ;base 0:15
		db 0x00   ;base 16:23
		db 10011010b ;access byte
		db 0xCF   ;flags & limit 16:19
		db 0x00   ;base 24:31
		;data descriptor 0x10, base 0, limit 0xffffffff, type 0x92
		dw 0xFFFF ;limit 0:15
		dw 0x0000 ;base 0:15
		db 0x00   ;base 16:23
		db 10010010b ;access byte
		db 0xCF   ;flags & limit 16:19
		db 0x00   ;base 24:31
		;video descriptor 0x18, base 0x0000b800, limit 0x0000ffff, type 0x92
		dw 0xFFFF ;limit 0:15
		dw 0x8000 ;base 0:15
		db 0x0B   ;base 16:23
		db 10010010b ;access byte
		db 0xCF   ;flags & limit 16:19
		db 0x00   ;base 24:31
gdt_end:

