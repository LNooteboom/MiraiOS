mov al, 'E'
mov ah, 0x0E
int 0x10
lol: jmp lol

checkentry:	;al = entry number, ES = dir entries
		mov si, sect2name
		xor ah, ah
		;dir entry size = 32 bytes
		shl ax, 5
		mov bx, ax

	.loop:	lodsb
		or al, al
		je .true
		mov ah, [es:bx]
		cmp al, ah
		jne .false
		inc bx
		;mov al, ah
		;call hexprintbyte
		jmp .loop
		
	.false:	xor ax, ax
		;mov al, ah
		;call hexprintbyte
		ret
	.true:	;mov al, 0xAB
		;call hexprintbyte
		add bx, 0x0F
		mov ax, [es:bx]
		ret
		
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
	
getfile:	mov ax, INITSEG
		mov es, ax
		mov bx, NROFROOTDIRENTS/16
		mov ax, RESSECTORS+(NROFFATS * NROFFATSECTS)+1
		call loadsector
		mov al, [es:0000h]
		;call hexprintbyte
		mov al, 0
		xor cl, cl
	.loop:	inc cl
		cmp cl, NROFROOTDIRENTS
		je error
		mov al, cl
		call checkentry
		cmp ax, 0x0000
		je .loop
		push ax
		mov bp, sp
		jmp loadsecondstage
		
	error:	mov ax, err
		mov si, err
		call printf
		jmp $

loadsecondstage:
		;load fat table
		xor ah, ah
		mov al, RESSECTORS + 1
		mov bl, NROFFATSECTS
		call loadsector
		;jump to clusternr in stack
		mov ax, [ss:bp] ;clusternr
		call getposfromcluster
		mov bx, ax
		jnc .inpos1
		jmp .inpos2

	.inpos1:mov al, [es:bx]
		mov ah, [es:bx+1]
		and ax, 0x0FFF
		jmp .next

	.inpos2:add bx, 1
		mov al, [es:bx]
		mov ah, [es:bx+1]
		and ax, 0xFFF0
		shr ax, 4

	.next:	cmp ax, 0
		je error
		mov bx, 1
		mov ax, SYSSEG
		mov es, ax
		mov ax, [ss:bp]
		add ax, RESSECTORS+(NROFFATSECTS*NROFFATS)+(NROFROOTDIRENTS/16)-1
		call loadsector
		;mov al, [es:0]
		;call hexprintbyte
		pop ax
		jmp far [es:0]

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