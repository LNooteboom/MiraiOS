
SECTION .text
extern sprint

global ps2SendPort1:function
global ps2SendCommand:function
global ps2ReadData:function
global ps2WriteData:function
global ps2ReadStatus:function

ps2ReadWait:	;(void) returns void
		in al, 0x64
		test al, 0x01
		jz ps2ReadWait
		ret

ps2WriteWait:	;(void) returns void
		in al, 0x64
		test al, 0x02
		jnz ps2WriteWait
		ret

ps2SendPort1:	;(char data) returns void
		push ebp
		mov ebp, esp

		mov ecx, 0x0000FFFF ;timeout value
	.start:	or ecx, ecx
		jz .err
		dec ecx
		in al, 0x64
		test al, 0x02
		jnz .start

		mov al, [ss:ebp+8]
		out 0x60, al
		leave
		ret

	.err:	mov eax, timeoutErr
		push eax
		call sprint
		add esp, 8
		jmp $

ps2SendCommand:;(char command) returns void
		push ebp
		mov ebp, esp

		call ps2WriteWait
		mov al, [ss:ebp+8]
		out 0x64, al

		leave
		ret

ps2ReadData:	;(void) returns char data
		call ps2ReadWait
		in al, 0x60
		ret

ps2WriteData:	;(char data) returns void
		push ebp
		mov ebp, esp

		call ps2WriteWait
		mov al, [ss:ebp+8]
		out 0x60, al

		leave
		ret

ps2ReadStatus:;(void) returns char status
		in al, 0x64
		ret

SECTION .rodata
timeoutErr:	db 'PS/2 device timed out.', 0
