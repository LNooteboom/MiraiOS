LOWMEM_SZ:	equ 0x100000 ;1mb

SECTION .text

global init_memory:function
init_memory:	;(void) returns void
		push ebp
		mov ebp, esp
		sub esp, 0x04

		mov ax, es
		mov [ss:bp-4], ax
		mov ax, ds
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
		mov [ds:0x1000], eax ;and store it

		;now fill kernel page table ...
		mov ecx, 1024
		mov eax, 0x00000000 ;nothing allowed
		mov edi, 0x2000
		cld
		rep stosd

		;setup 1:1 paging in lowmem for kernel
		xor ecx, ecx
		mov edi, 0x2000

	.start:	cmp ecx, LOWMEM_SZ
		jge .cont
		;add to page table
		mov eax, ecx
		mov al, 0x07
		stosd

		add ecx, 0x1000 ;4kb
		jmp .start


	.cont:	mov eax, 0x1000
		mov cr3, eax

	.next:	;now enable paging
		mov eax, cr0
		or eax, 0x80000000
		mov cr0, eax

	.n:	mov ax, [ss:bp-4]
		mov es, ax
		mov esp, ebp
		pop ebp
		ret

global movemem:function
movemem:	;(char *startaddr, int nrofbytes, int offset) returns void
		push ebp
		mov ebp, esp

		mov eax, [ss:ebp+0x10]
		mov esi, eax
		add eax, [ss:ebp+0x08]
		mov edi, eax

		mov ecx, [ss:ebp+0x0C]

		rep movsb

		leave
		ret

global TLB_update:function
TLB_update:	mov eax, cr3
		mov cr3, eax
		ret

SECTION .data

krnloff:	dd 0x00007000
stackoff:	dd 0x9c00

