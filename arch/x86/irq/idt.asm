BITS 32

global initIDT:function
global routeInterrupt:function
global unrouteInterrupt:function

global undefinedInterrupt:function

NROFIDTENTS:	equ 0x100
BYTESPERIDTENT:	equ 0x08

SECTION .text

initIDT:	;(void) returns void
		push ebp
		mov ebp, esp

		push edi

		cld
		xor cx, cx
		mov edi, IDT

	.start:	cmp cx, NROFIDTENTS
		je .cont
		mov eax, undefinedInterrupt
		stosw ;offset 0:15
		mov ax, 0x08
		stosw ;selector in GDT
		mov al, 0x00
		stosb ;always zero
		mov al, 0x8F
		stosb ;type and attributes
		shr eax, 16
		stosw ;offset 16:31

		inc cx
		jmp .start

	.cont:
		mov eax, IDT
		;sub eax, 0xC0000000
		;mov [idtr+2], eax
		lidt [idtr]
		;sti

		pop edi
		leave
		ret

undefinedInterrupt:
		iret

routeInterrupt: ;(void (*ISR)(void), uint8_t interrupt, uint8_t flags) returns void
		;flags bit 0 = Interrupt when clear, trap when set
		push ebp
		mov ebp, esp
		push edi

		mov al, [ebp+12]
		mov cl, BYTESPERIDTENT
		mul cl

		movzx edi, ax
		add edi, IDT
		
		mov eax, [ebp+8]
		;xchg bx, bx
		stosw
		mov ax, 0x08
		stosw
		mov al, 0x00
		stosb

		mov al, [ebp+16]
		test al, 0x01
		jnz .trap
		mov al, 0x8E
		jmp .end
	.trap:	mov al, 0x8F

	.end:	stosb

		shr eax, 16
		stosw


		pop edi
		pop ebp
		ret

unrouteInterrupt: ;(uint8_t interrupt) returns void
		push ebp
		mov ebp, esp

		mov al, [ebp+8]
		mov dl, BYTESPERIDTENT
		mul dl

		movzx edx, ax
		xor al, al
		add edx, (IDT + 5)
		mov [edx], al

		pop ebp
		ret

SECTION .data

idtr:	dw (NROFIDTENTS)
	dd IDT;0 ;(IDT - 0xC0000000)

SECTION .bss align=4096
IDT:	resb (NROFIDTENTS * BYTESPERIDTENT)
