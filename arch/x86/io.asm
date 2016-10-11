;includes <io>

SECTION .text
global inb:function
global outb:function

inb:	;(uint16_t port) returns uint8_t value
	mov dx, [esp+4]
	in al, dx
	ret

outb:	;(uint16_t port, uint8_t value) returns void
	mov dx, [esp+4]
	mov al, [esp+8]
	out dx, al
	ret
