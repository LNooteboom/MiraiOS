SECTION .text

global PIT_init:function
PIT_init:	;(void) returns void
		pushfd
		cli
		mov al, 00110100b
		out 0x43, al
		mov ax, 0x0020
		out 0x40, al
		mov al, ah
		out 0x40, al
		popfd
		ret
