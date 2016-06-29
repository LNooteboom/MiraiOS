
SECTION .text
global inb:function
inb:		;(short port)
		push ebp
		mov ebp, esp
		xor eax, eax
		mov dx, [ebp + 8]
		in al, dx
		pop ebp
		ret

global outb:function
outb:		;(short port, char value)
		push ebp
		mov ebp, esp
		mov ax, [ebp + 8]
		mov dx, [ebp + 12]
		out dx, ax
		pop ebp
		ret

