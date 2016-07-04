;all functions here use the cdecl calling convention unless specified otherwise
BITS 32

extern linewidth
extern hexprint

SECTION .text

;global write_to_vram:function
;write_to_vram:	;(uint8_t value, uint16_t offset)
;		;es must be set to vram first
;		push ebp
;		mov ebp, esp
;		mov al, [ebp + 8] ;value
;		mov ebx, [ebp + 12] ;address
;		mov [es:ebx], al
;		pop ebp
;		ret

global video_init:function
video_init:	;(void) returns void
		call getCRTCPorts
		
		ret

getCRTCPorts:	;(void) returns void
		mov dx, 0x3CC ;misc output port
		in al, dx
		test al, 0x01 ;test first bit
		jnz .end ;leave to default
		mov ax, 0x3B4
		mov [CRTC_index_port], ax
		mov ax, 0x3B5
		mov [CRTC_value_port], ax
	.end:	ret
		

global get_line_width:function
get_line_width:	;(void) returns int width
		push ebp
		mov ebp, esp

		;save the old value in the address register
		mov dx, [CRTC_index_port]
		in al, dx
		push eax
		;first we need to find out what mode is currently enabled
		;check if dword mode bit is set
		mov al, 0x14
		out dx, al
		mov dx, [CRTC_value_port]
		in al, dx
		test al, 0x40
		jnz .not_dword
		mov cl, 2
		jmp .cont

	.not_dword:
		mov al, 0x17
		mov dx, [CRTC_index_port]
		out dx, al
		mov dx, [CRTC_value_port]
		in al, dx
		test al, 0x40 ;0b01000000
		jz .byte
	.word:	mov cl, 1
		jmp .cont
	.byte:	mov cl, 0
	.cont:	mov al, 0x13
		mov dx, [CRTC_index_port]
		out dx, al
		mov dx, [CRTC_value_port]
		xor eax, eax
		in al, dx
		shl eax, cl
		;shr eax, 2
		mov edx, eax
		pop eax
		push edx
		mov dx, [CRTC_index_port]
		out dx, al
		pop eax
		pop ebp
		ret

global vga_set_cursor:function
vga_set_cursor:	;(int cursorX, int cursorY) returns void
		;updates VGA cursor position
		push ebp
		mov ebp, esp
		;mov edx, [ss:ebp+8]
		mov eax, [ss:ebp+12]
		mul dword [linewidth]
		shr eax, 1
		;push eax
		;call hexprint
		;jmp $
		mov edx, eax
		mov eax, [ss:ebp+8]
		;shl eax, 1
		add eax, edx
		mov [ss:ebp+8], eax
		;save old index port
		mov dx, [CRTC_index_port]
		in al, dx
		mov [ss:ebp+12], al
		;low byte
		mov al, 0x0F
		out dx, al
		mov dx, [CRTC_value_port]
		mov al, [ss:ebp+8]
		out dx, al
		;high byte
		mov al, 0x0E
		mov dx, [CRTC_index_port]
		out dx, al
		mov al, [ss:ebp+9]
		mov dx, [CRTC_value_port]
		out dx, al

		pop ebp
		ret

SECTION .data
CRTC_index_port: dw 0x03D4
CRTC_value_port: dw 0x03D5

