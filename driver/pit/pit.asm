SECTION .text

extern counter
extern timer_enabled
extern hexprintln

global PIT_init:function
PIT_init:	;(void) returns void
		pushfd
		cli
		mov al, 00110110b
		out 0x43, al
		mov ax, 1193 ;1000.152hz
		out 0x40, al
		mov al, ah
		out 0x40, al
		popfd
		ret

global sleep:function
sleep:		;(int time_in_ms) returns void
		push ebp
		mov ebp, esp

		;set delay
		mov eax, [ss:ebp+8]
		mov [counter], eax
		push eax
		call hexprintln
		add esp, 4

		;turn on timer
		mov al, 1
		mov [timer_enabled], al

		;now wait until timer is disabled
	.loop:	mov al, [timer_enabled]
		or al, al
		jnz .loop

		leave
		ret
