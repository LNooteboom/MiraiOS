ORG	0x10000
nop
nop
jmp start
CURRENTSEG	equ	0x1000
EXTRASEG	equ	0x7000

SECTPERTRACK	equ	0x0012
NROFHEADS	equ	0x0002
RESSECTORS	equ	0x0001
NROFFATS	equ	0x0002
NROFFATSECTS	equ	0x0009
NROFROOTDIRENTS	equ	0x00E0


start:		mov bl, drivenumber
		;pop ax
		call hexprintbyte
		push ax
		mov ax, EXTRASEG
		mov es, ax
		call loadfattable
		pop ax

	.loop:	push ax
		;add ax, RESSECTORS+(NROFFATSECTS*NROFFATS)+(NROFROOTDIRENTS/16)-1
		;call loadsector
		pop ax
		call getfatentry
		cmp ax, 0x0FF8
		jge .eof
		cmp ax, 0x0FF0
		jge .err
		cmp ax, 0x0001
		jle .err
		jmp .loop

	.err:	;pointing to bad/reserved sector

	.eof:	jmp $

hexprintbyte:	push bp
		mov bp, sp
		push ax
		and al, 0xF0
		shr al, 4
		call translatenibble
		mov ah, 0x0E
		int 0x10
		
		mov ax, [es:bp+2]
		and al, 0x0F
		call translatenibble
		mov ah, 0x0E
		int 0x10
		mov al, ' '
		int 0x10
		pop ax
		pop bp
		ret
		
translatenibble:
		cmp al, 10
		jge hexchar
		;0-9
		add al, '0'
		ret
	hexchar: ; A-F
		add al, 'A' - 10
		ret

loadfattable:	xor ah, ah
		mov al, RESSECTORS + 1
		mov bl, NROFFATSECTS
		call loadsector
		;mov al, [es:0]
		;call hexprintbyte
		ret

getfatentry:	;ax = clusternr, fat table in es, returns ax = table entry value
		push bp
		mov bp, sp
		call getposfromcluster
		mov bx, ax
		jc .pos2

	.pos1:	mov al, [es:bx]
		mov ah, [es:bx+1]
		and ax, 0x0FFF
		jmp .end

	.pos2:	mov al, [es:bx+1]
		mov ah, [es:bx+2]
		and ax, 0xFFF0
		shr ax, 4

	.end:	pop bp
		
getposfromcluster: ;ax = clusternr, carry = offset 0 or 1
		sub ax, 1
		push ax
		shr ax, 1
		mov bx, ax
		pop ax
		pushf
		;add bx, ax
		add ax, bx
		xor bx, bx
		popf
	.end:	ret	

loadsector:	;sector number in AX, result in ES:0000h, sectors to read in bl
		push bx
		push ax
		mov ah, 0
		mov dl, [drivenumber]
		int 0x13
		
		pop ax
		mov bl, SECTPERTRACK*NROFHEADS
		div bl
		mov ch, al	;track

		mov al, ah
		xor ah, ah
		mov bl, SECTPERTRACK
		div bl
		mov dh, al	;head number
		mov cl, ah	;sector

		pop bx
		mov al, bl	;sectors to read
		mov bx, 0000h	;dest. memory offset
		mov ah, 02h	;instruction code
		mov dl, [drivenumber]
		int 0x13
		;call movemem
		ret

drivenumber:	db	0
