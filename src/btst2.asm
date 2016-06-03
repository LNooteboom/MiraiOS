ORG	0x10000
CURRENTSEG	equ	0x1000
FATSEG		equ	0x7000

SECTPERTRACK	equ	0x0012
NROFHEADS	equ	0x0002
RESSECTORS	equ	0x0001
NROFFATS	equ	0x0002
NROFFATSECTS	equ	0x0009
NROFROOTDIRENTS	equ	0x00E0


start:		push ax
		mov ax, CURRENTSEG
		mov ds, ax
		mov [drivenumber], bl
		pop ax
		mov [currentsector], ax
		mov ax, FATSEG
		mov fs, ax
		mov es, ax
		call loadfattable
		mov ax, CURRENTSEG
		mov es, ax
		mov ax, [currentsector]
		xor bx, bx

	.loop:	push bx
		push ax
		add ax, RESSECTORS+(NROFFATSECTS*NROFFATS)+(NROFROOTDIRENTS/16)-1
		mov cl, 1
		call loadsector
		pop ax
		call getfatentry
		;call hexprintbyte
		cmp ax, 0x0FF8
		jge .eof
		cmp ax, 0x0FF0
		jge .err
		cmp ax, 0x0001
		jle .err
		pop bx
		add bh, 0x02
		jmp .loop

	.err:	;pointing to bad/reserved sector
		mov si, badsectormsg
		call print
		jmp $

	.eof:	mov si, stage2loadedmsg
		call print
		jmp next

hexprintbyte:	push bp
		mov bp, sp
		push ax
		and al, 0xF0
		shr al, 4
		call translatenibble
		mov ah, 0x0E
		int 0x10
		
		mov ax, [ss:bp-2]
		;pop ax
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
		jge .hexchar
		;0-9
		add al, '0'
		ret
	.hexchar: ; A-F
		add al, 'A' - 10
		ret

print:		lodsb
		or al, al
		jz .done
		mov ah, 0x0E
		int 0x10
		jmp print
	.done: 	ret

loadfattable:	mov ax, RESSECTORS + 1
		;mov ax, 1
		mov cl, NROFFATSECTS
		xor bx, bx
		call loadsector
		;mov al, [fs:0]
		;call hexprintbyte
		ret

getfatentry:	;ax = clusternr, fat table in fs, returns ax = table entry value
		call getposfromcluster
		mov bx, ax
		jc .pos2

	.pos1:	mov al, [fs:bx]
		mov ah, [fs:bx+1]
		and ax, 0x0FFF
		jmp .end

	.pos2:	mov al, [fs:bx+1]
		mov ah, [fs:bx+2]
		and ax, 0xFFF0
		shr ax, 4

	.end:	ret
		
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

loadsector:	;sector number in AX, result in ES:BX, sectors to read in cl
		push bx
		push cx
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
		pop bx
		mov ah, 02h	;instruction code
		mov dl, [drivenumber]
	.retry:	int 0x13
		jc .retry
		;mov al, [es:bx]
		;call hexprintbyte
		ret

drivenumber:	db	0
currentsector:	dw	0
stage2loadedmsg:db	"Loaded stage 2", 13, 10, 0
badsectormsg:	db	"ERROR: bad sector detected!", 13, 10, 0

next:		call test_a20
		call hexprintbyte
		call init_a20
		call test_a20
		call hexprintbyte

test_a20:	pushf
		push ds
		push es
		push di
		push si

		cli
		
		xor ax, ax
		mov es, ax

		not ax
		mov ds, ax

		mov ax, [es:0x7DFE]
		cmp ax, [ds:0x7E0E]
		jne .yes
		mov ax, 0xABCD
		mov [es:0x7DFE], ax
		cmp ax, [ds:0x7E0E]
		jne .yes
		mov ax, 0
		jmp .end

	.yes:	mov ax, 1


	.end:	pop si
		pop di
		pop es
		pop ds
		popf
		ret

init_a20:	push ax
		cli

		call kyb_wait_in
		mov al, 0xAD
		out 0x64, al ;disable input

		call kyb_wait_in
		mov al, 0xD0
		out 0x64, al

		call kyb_wait_out
		in al, 0x60
		push ax

		call kyb_wait_in
		mov al, 0xD1
		out 0x64, al

		call kyb_wait_in
		pop ax
		or al, 2
		out 0x60, al

		call kyb_wait_in
		mov al, 0xAE
		out 0x64, al

		call kyb_wait_in
		sti
		pop ax
		ret


kyb_wait_in:	in al, 0x64 ;read status
		test al, 2 ;check if input buffer flag is clear
		jnz kyb_wait_in
		ret

kyb_wait_out:	in al, 0x64
		test al, 1
		jz kyb_wait_out
		ret
