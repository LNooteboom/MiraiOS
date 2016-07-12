
SECTION .text
extern sprint
extern currentattrib

ps2_read_wait:	;(void) returns void
		in al, 0x64
		test al, 0x01
		jz ps2_read_wait
		ret

ps2_write_wait:	;(void) returns void
		in al, 0x64
		test al, 0x02
		jnz ps2_write_wait
		ret

global ps2_send_port1:function
ps2_send_port1:	;(char data) returns void
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

	.err:	mov eax, [currentattrib]
		push eax
		mov eax, timeouterr
		push eax
		call sprint
		add esp, 8
		jmp $

global ps2_send_command:function
ps2_send_command:;(char command) returns void
		push ebp
		mov ebp, esp

		call ps2_write_wait
		mov al, [ss:ebp+8]
		out 0x64, al

		leave
		ret

global ps2_read_data:function
ps2_read_data:	;(void) returns char data
		call ps2_read_wait
		in al, 0x60
		ret

global ps2_write_data:function
ps2_write_data:	;(char data) returns void
		push ebp
		mov ebp, esp

		call ps2_write_wait
		mov al, [ss:ebp+8]
		out 0x60, al

		leave
		ret

global ps2_read_status:function
ps2_read_status:;(void) returns char status
		in al, 0x64
		ret

SECTION .data
timeouterr:	db 'PS/2 device timed out.', 0
