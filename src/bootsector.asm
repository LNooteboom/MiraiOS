BOOTSEG	equ	0x7c00
INITSEG	equ	0x9000
SYSSEG	equ	0x1000
SYSSIZE	equ	0x1000
ENDSEG	equ	SYSSEG + SYSSIZE

SECTPERTRACK	equ	0x0012
NROFHEADS	equ	0x0002
RESSECTORS	equ	0x0001
NROFFATS	equ	0x0002
NROFFATSECTS	equ	0x0009
NROFROOTDIRENTS	equ	0x00E0

		ORG BOOTSEG
		;first we need to setup the BIOS parameter block:
		;the first 3 bytes must be a short jmp and a nop
		jmp short bootloader
		nop
		;then we get an 8 byte OEM id, for a linux formatted disk, this contains mkdosfs(space)
		db 'mkdosfs '
		;next we need to define the number of bytes per sector, in this case 512
bpersect:	dw 0x0200
		;now we need to define the amount of sectors per cluster, in this case (3.5 HD floppy) set this to 1
sectpercl:	db 0x01
		;number of reserved sectors, this value must at least be 1 for the bootsector
nrressect:	dw RESSECTORS
		;number of file allocation tables, almost always 2
nrofFATs:	db NROFFATS
		;number of root directory entries????
nrrootdirent:	dw NROFROOTDIRENTS
		;total logical sectors
nrofsect:	dw 2880
		;media descriptor
med_desc:	db 0xF0
		;sectors for FAT
nrFATsects:	dw NROFFATSECTS
		;number of sectors per track
		dw 0x0012
		;number of heads
		dw 0x0002
		;number of hidden sectors
nrofhid:	dd 0x00000000
		;large amount of sectors
		dd 0x00000000
		;drive number
		db 0x00
		;windows NT flags
		db 0x00
		;signature
		db 0x29
		;volumeID serial number (random I guess)
		dd 0x130C2E45
		;volume label 11 chars
vollabel:	db 'LukeOS Boot'
		;system identifier string 8 chars
		db 'FAT12   '
		
		;now for the boot code:
bootloader:
		xor ax, ax
		mov ds, ax
		mov ss, ax
		mov ax, 0x0900
		mov es, ax
		mov sp, 0x9c00
		
		mov [drivenumber], dl

		cld
		
		mov si, msg
		call printf
		;now load the system into the memory
		;jmp loadsecondstage
		call getfile
		
		
		hang: jmp hang
		
printf:		lodsb
		or al, al
		jz .done
		mov ah, 0x0E
		int 0x10
		jmp printf
	.done: 	ret

hexprintbyte:
		push ax
		and al, 0xF0
		shr al, 4
		call translatenibble
		mov ah, 0x0E
		int 0x10
		
		pop ax
		and al, 0x0F
		call translatenibble
		mov ah, 0x0E
		int 0x10
		mov al, ' '
		int 0x10
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
		
loadsecondstage:
		;jmp 0x1000
		
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

movemem:
		mov ax, 0x0100
		mov gs, ax
		mov bx, 0
repeatw:	mov ax, [es:bx]
		mov [gs:bx], ax
		add bx, 1
		cmp bx, 0x0100
		jne repeatw
		ret

getfile:	mov ax, INITSEG
		mov es, ax
		mov bx, NROFROOTDIRENTS/16
		mov ax, RESSECTORS+(NROFFATS * NROFFATSECTS)+1
		call loadsector
		mov al, [es:0000h]
		;call hexprintbyte
		mov cl, 5
		xor al, al

	.loop:	
		call checkentry
		mov al, 1
		call checkentry
		ret
		
		

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
		mov al, ah
		call hexprintbyte
		jmp .loop
		
	.false:	xor al, al
		mov al, ah
		;call hexprintbyte
		ret
	.true:	mov al, 0xAB
		call hexprintbyte
		ret
		
msg		db 'Welcome to LN-DOS  boot loader version 0.02!', 13, 10, 0
msg2:		db 'Loaded second stage', 13, 10, 0
sect2name	db 'BTST2   BIN', 0
drivenumber:	db 0
		times 510 - ($ - $$) db 0
		db 0x55
		db 0xAA
sect1:		;sector 1
		;mov si, msg2
		; call printf
; lol:	jmp lol
		
		;FAT TABLE 1
		db 0xF0
		db 0xFF
		db 0xFF
		;cluster 1
		db 0xFF
		db 0x0F
		db 0x00
		
		times 5120 - ($ - $$) db 0
		;FAT TABLE 2
		db 0xF0
		db 0xFF
		db 0xFF
		
		db 0xFF
		db 0x0F
		db 0x00
		
		
		
		times 0x2600 - ($ - $$) db 0
		;root dir
		db 'LN-DOS     ' ;dir name
		db 0x08 ;directory+volume
		db 0x00 ;NT flags
		db 0x00 ;creation time in 10ths of a second
		dw 0x0000 ;creation time
		dw 0x0000 ;creation date
		dw 0x0000 ;access date
		dw 0x0000 ;high word of cluster nr not used in FAT12/16
		dw 0x74c7 ;mod time
		dw 0x48B0 ;mod date
		dw 0x0000 ;cluster
		dd 0x00000000 ;file size
		
		
		times 1474560 - ($ - $$) db 0
