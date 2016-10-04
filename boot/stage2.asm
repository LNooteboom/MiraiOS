BITS 16
ORG	0x0500
CURRENTSEG	equ	0x0000
FATSEG		equ	0x7000
KRNL_LOADADDR	equ	0x10000
KRNL_BUFFERADDR	equ	0x7C00
KRNL_ELFADDR	equ	0x7E00
KRNL_PROGHEADADDR equ	0x7E00
ROOTDIR_ADDR	equ	0x7C00

SECTPERTRACK	equ	0x0012
NROFHEADS	equ	0x0002
RESSECTORS	equ	0x0001
NROFFATS	equ	0x0002
NROFFATSECTS	equ	0x0009
NROFROOTDIRENTS	equ	0x00E0

FILEOFFSET	equ	RESSECTORS+(NROFFATSECTS*NROFFATS)+(NROFROOTDIRENTS/16)-1
PARAM_TABLE_SEG	equ	0x0300


%define endl	13,10,0

global _stage2
_stage2:	
		push ax
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
		mov cx, 1
		xor dx, dx
		mov bx, 0x0700
		call followfat

		mov si, stage2loadedmsg
		call print
		jmp next

followfat:	;ax = first sector, cx = sectors to skip from start
		;dx = amount of sectors to read 0 = 65536, stops at FAT EOF marker
		;result in [es:bx]
		push bp
		mov bp, sp

	.loop:	push bx
		push ax
		cmp cx, 0
		jne .noload
		push cx
		push dx
		add ax, FILEOFFSET
		mov cl, 1
		call loadsector
		pop dx
		pop cx
		dec dx
		cmp dx, 0
		je .end
		jmp .cont

	.noload:;sub bh, 0x02
		;dec cx

	.cont:	pop ax
		call getfatentry ;should only change ax and bx
		cmp ax, 0x0FF8
		jge .eof
		cmp ax, 0x0FF0
		jge .err
		cmp ax, 0x0001
		jle .err
		pop bx
		jcxz .cont2
		dec cx
		jmp .loop
	.cont2:	add bh, 0x02
		jmp .loop

	.err:	;pointing to bad/reserved sector
		mov si, badsectormsg
		call print
		jmp $

	.end:	pop ax
	.eof:	pop bx
		pop bp
		ret

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
		;pushf
		;mov al, [fs:bx]
		;call hexprintbyte
		;mov al, [fs:bx+1]
		;call hexprintbyte
		;popf
		jc .pos2

	.pos1:	mov al, [fs:bx]
		mov ah, [fs:bx+1]
		and ax, 0x0FFF
		jmp .end

	.pos2:	mov al, [fs:bx]
		mov ah, [fs:bx+1]
		and al, 0xF0
		shr ax, 4

	.end:	ret
		
getposfromcluster: ;ax = clusternr, carry = offset 0 or 1
		push ax
		shr ax, 1
		mov bx, ax
		pop ax
		pushf
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
		dec ax
		mov bl, SECTPERTRACK*NROFHEADS
		div bl
		mov ch, al	;track

		mov al, ah
		xor ah, ah
		mov bl, SECTPERTRACK
		div bl
		mov dh, al	;head number
		mov cl, ah	;sector
		inc cl

		pop bx
		mov al, bl	;sectors to read
		pop bx
		mov ah, 02h	;instruction code
		mov dl, [drivenumber]
	.retry:	int 0x13
		jc .retry
		ret


drivenumber:	db	0
currentsector:	dw	0
stage2loadedmsg:db	"Loaded stage 2", endl
badsectormsg:	db	"ERROR: bad sector detected!", endl

;This whole damn bootloader is the definition of the dutch word 'krakkemikkig'
;I really don't trust it.

next:		call test_a20 ;test if A20 is already enabled
		or al, al
		jnz .a20en
		call init_a20 ;if not, enable A20 via the keyboard controller
		call test_a20
		or al, al
		jz .a20err
	.a20en:	mov si, a20enabledmsg
		call print
		jmp .next0
	.a20err:mov si, a20errormsg
		call print
		jmp $
	.next0:	;jmp $
		call unreal_mode
		call load_krnl
		;turn off floppy drive motor
		mov dx, 0x3F2
		in al, dx
		and al, 0x0F
		out dx, al

		call getparameters
		mov ax, [krnlentry]
		call hexprintbyte
		mov al, ah
		call hexprintbyte

		call init_paging
		call pmode
		mov ax, 0x18
		mov es, ax
		xor bx, bx
		;mov [es:00], byte 'K'
		mov eax, [krnlentry]
		;mov [jmpfar + 2], eax ;self modifying code

		xor eax, eax
		mov ax, sp
		add eax, 0xC0000000
		mov esp, eax

		;Now we hand the system over to the kernel
		mov ax, 0x0010
		mov ss, ax
		mov fs, ax
		mov gs, ax
		mov ds, ax
		mov ax, 0x0018
		mov es, ax
	jmpfar:	jmp 8:dword 0xC0100000 ;bye

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

unreal_mode:	push es
		push ds

		lgdt [gdtr]
		;switch to pmode
		mov eax, cr0
		or al, 0x01
		mov cr0, eax

		jmp $+2

		mov bx, 0x10 ;data seg
		mov ds, bx
		mov es, bx

		;switch back to real mode
		and al, 0xFE
		mov cr0, eax

		pop ds
		pop es

		ret

load_krnl:	
		mov bx, ROOTDIR_ADDR
		mov ax, RESSECTORS + (NROFFATS * NROFFATSECTS) + 1
		mov cl, NROFROOTDIRENTS/16
		call loadsector

		;Root dir entries are now loaded
		xor cx, cx
	.start:	cmp cx, NROFROOTDIRENTS
		je .nfound ;if it has run out of entries to look in
		mov bx, cx
		shl bx, 5 ;times 32 to get address
		add bx, ROOTDIR_ADDR
		mov si, krnlfilename
	.start2:mov al, bl
		and al, 0x1F ;31
		cmp al, 0x0B ;if all chars have been compared
		je .found
		lodsb
		inc bl
		cmp al, [es:bx-1]
		je .start2
	.end2:	inc cx
		jmp .start
	
	.nfound:mov si, krnlnfounderror
		call print
		jmp $

	.found:	and bl, 0xE0 ;ignore lowest 5 bits
		add bx, 0x1A ;start of file cluster nr
		mov ax, [es:bx]
		mov [krnlsector], ax ;save it

		mov si, krnlfoundmsg
		call print

		;mov ax, KRNL_ELFADDR
		;mov es, ax
		;fat table should still be loaded in fs

		mov ax, [krnlsector]
		add ax, FILEOFFSET
		;clc
		;call followfat
		;first we need to load in the first cluster to interpret the ELF header
		mov bx, KRNL_ELFADDR
		mov cl, 1
		call loadsector
		;ok now we need to verify if the 'magic' bytes are correct
		cmp [KRNL_ELFADDR], word 0x457F ;0x7F & 0x45 ('E')
		jne .ferror
		cmp [KRNL_ELFADDR+0x02], word 0x464C ;0x4C & 0x46 ('LF') remember, we use little endian
		jne .ferror
		;now that the magic bytes are verified, we need to make sure the file is
		;32 bit and little endian (other modes are not supported)
		cmp [KRNL_ELFADDR+0x04], word 0x0101
		jne .ferror ;I should probably make more error messages, oh well
		;Now check the instruction set
		cmp [KRNL_ELFADDR+0x12], word 0x0003 ;x86
		jne .ferror

		;Let's now get the program header
		;make sure that the 9 most significant bits are cleared
		cmp [KRNL_ELFADDR+0x1F], byte 0x00
		jne .ferror
		test [KRNL_ELFADDR+0x1E], byte 0x80
		jnz .ferror
		;now get the program header position, the least significant byte doesnt matter
		;as we are just looking at the cluster it is in
		mov ax, [KRNL_ELFADDR+0x1D] ;here we are looking into the 2 middle bytes
		shr ax, 1 ;divide it by 2 to get relative cluster nr
		mov [progheadcluoffset], ax
		;now we need to get the offset from the cluster in bytes
		mov ax, [KRNL_ELFADDR+0x1C]
		and ax, 0x01FF ;only the lowest 9 bits matter
		mov [progheadboffset], ax
		;as well as the number of entries
		mov ax, [KRNL_ELFADDR+0x2C]
		mov [progheadsize], ax
		;now get the total program header table size in bytes
		mov dx, [KRNL_ELFADDR+0x2E]
		mul dx ;and multiply it to get the size in bytes
		mov [progheadsizeb], ax
		shr ax, 9 ;divide by 512 for the amount of clusters
		inc al
		mov [progheadsizecluster], al
		;and we also need the size 1 entry takes up
		mov ax, word [KRNL_ELFADDR+0x2A]
		mov [progheadentsize], ax

		;store entry offset:
		mov eax, [KRNL_ELFADDR+24]
		mov [krnlentry], eax

		;now we can finally start loading in the program headers
		mov bx, KRNL_PROGHEADADDR
		mov cx, [progheadcluoffset]
		mov dl, [progheadsizecluster]
		xor dh, dh
		mov ax, [krnlsector]
		call followfat

		mov bx, [progheadboffset]
		add bx, KRNL_PROGHEADADDR
		mov cx, [progheadsize]
		;now we have to decode the header entries
	.start3:jcxz .succ
		mov al, [es:bx]
		cmp al, 0
		je .end3 ;skip null header
		cmp al, 1
		je .cont
		call .perror
		jmp .end3 ;skip it

	.cont:	push cx
		push bx
		call progheaddecode
		pop bx
		pop cx

	.end3:	add bx, 0x0020
		dec cx
		jmp .start3
		
	.perror:mov si, progheadnoload
		call print
		jmp $

	.ferror:mov si, krnlformaterror
		call print
		jmp $

	.succ:	mov si, krnlloadedmsg
		call print

		ret

progheaddecode:	;this function decodes a program header table entry and executes it
		push bp
		mov bp, sp
		;push 0x10000
		mov edi, 0x100000 ;edi = current offset in pmem
		;stack map:
		;+4h program header offset (please preserve)
		;first load in the file at the current offset into the bufferxit
	.start:	mov bx, [ss:bp+0x04]
		mov eax, [bx+4] ;p_offset
		xor edx, edx
		mov ecx, 0x200
		div ecx
		push dx
		mov cx, ax
		mov bx, KRNL_BUFFERADDR
		mov dx, 1
		mov ax, [krnlsector]
		call followfat
		
		;now copy it from the buffer to the desired mem location
		pop ax
		mov bx, [ss:bp+0x04]
		mov ecx, [bx+16] ;p_filesz
		cmp ecx, 0x200
		jbe .lower
		mov ecx, 0x200
	.lower:	sub cx, ax
		movzx esi, ax
		add esi, KRNL_BUFFERADDR
		;subtract buffer size from filesize
		mov eax, [bx+16]
		sub eax, ecx
		mov [bx+16], eax
		;and add it to file offset
		mov edx, [bx+4]
		add edx, ecx
		mov [bx+4], edx

	.loop:	or cx, cx
		jz .end
		mov al, [esi]
		inc esi
		dec cx
		mov [edi], al
		inc edi
		jmp .loop

	.end:	mov eax, [bx+16]
		or eax, eax
		jz .exit
		jmp .start

	.exit:	leave
		ret

getparameters:	;store them in a table
		push bp
		mov bp, sp
		sub sp, 4
		mov ax, PARAM_TABLE_SEG
		mov es, ax
		xor ax, ax
		mov di, ax
		;kernel offset and memsize
		mov eax, 0x100000
		stosd
		mov eax, krnlmemsz
		stosd
		;cursor pos
		mov ah, 0x03
		xor bh, bh
		int 0x10
		mov al, dl
		stosb
		mov al, dh
		stosb
		;reserve memory table size
		;save di
		mov [ss:bp-2], di
		mov [ss:bp-4], word 0
		add di, 2
		;table pointer
		mov dx, di
		add di, 4
		xor ebx, ebx
		xor eax, eax
		mov bx, di
		mov ax, es
		shl eax, 4 ;times 16 for memory address
		add eax, ebx
		mov di, dx
		stosd

		;memory detection
		xor ebx, ebx
		mov edx, 0x534D4150 ;magic number
	.start:	
		inc word [ss:bp-4]
		mov eax, 0xE820
		mov ecx, 24
		int 0x15
		jc .end
		cmp eax, 0x534D4150 ;more magic number
		jne .bioserr
		mov ax, di
		;call hexprintbyte
		or ebx, ebx
		jz .end
		add di, 24
		jmp .start

	.bioserr:
		mov eax, 0xdeadbeef
		jmp $

	.end:	;jmp $
		push di
		mov di, [ss:bp-2]
		mov ax, [ss:bp-4]
		stosw
		pop di
		leave
		ret

init_paging:
		push bp
		mov bp, sp
		push es

		;fill page directory
		mov ax, 0x100
		mov es, ax
		mov di, 0
		mov cx, 1024*2 ;number of entries
		xor ax, ax
		cld
		rep stosw

		;now add entry
		xor di, di
		;point entry to 0x2000
		mov ax, 0x2009 ;add flags also
		stosw
		;high address
		xor ax, ax
		stosw

		;fill page table
		mov cx, 1024*2
		mov ax, 0x0200
		mov es, ax
		xor di, di
		xor ax, ax
		rep stosw

		;setup 1:1 lowmem paging
		xor cx, cx
		mov di, 0

	.start:	cmp cx, 0x100 ;amount of pages
		jge .next
		mov ax, cx
		shl ax, 12 ;shift to page nr field
		or al, 0x03 ;flags
		stosw
		mov ax, cx
		shr ax, 4
		stosw

		inc cx
		jmp .start

	.next:	;add page directory entry to itself
		mov eax, 0x1003
		mov [es:(0x1000 - 0x04)], eax ;it's the last entry
		;now setup higher half paging
		;add pointer in page directory
		;get offset
		mov ax, 0x0100
		mov es, ax
		mov ax, (0xC000 >> 4)
		mov di, ax
		;kernel page table is loaded at 0x4000
		mov ax, 0x4009 ;low addr + flags
		stosw
		xor ax, ax
		stosw ;high addr
		
		;fill the entire table (1024 entries)
		mov ax, 0x0400
		mov es, ax
		xor di, di
		xor cx, cx
	.start2:cmp cx, 1024
		jge .next2
		mov ax, cx
		shl ax, 12
		or al, 0x03 ;flags
		stosw
		mov ax, cx
		;add ax, 0x0100
		shr ax, 4
		stosw

		inc cx
		jmp .start2

	.next2:	mov eax, 0x1000
		mov cr3, eax

		pop es
		leave
		ret

pmode:		cli
		lgdt [gdtr]
		mov eax, cr0
		or eax, 0x80000001 ;enable pmode + paging
		mov cr0, eax

		ret

memE820err:	db 'WARN: Main memory dectection not available, trying alternatives...', endl
lowmemerror:	db 'WARN: Could not detect low memory, assuming 0x80000', endl

krnlmemsz:	dd 0
krnlgdtseg:	dw 8
krnlentry:	dd 0
krnlsector:	dw 0

a20enabledmsg:	db 'A20 line is enabled', endl
a20errormsg:	db 'ERROR: Could not enable A20', endl

krnlfilename:	db 'KERNEL     '
krnlnfounderror:db 'ERROR: File KERNEL could not be found!', endl
krnlfoundmsg:	db 'KERNEL found, loading...', endl
krnlloadedmsg:	db 'KERNEL loaded!', endl
krnlformaterror:db 'KERNEL format error', endl

progheadnoload:	db 'WARN: KERNEL Program header contains unsupported mode, skipping...', endl
progheadcluoffset:	dw 0
progheadboffset:dw 0
progheadsize:	dw 0
progheadsizeb:	dw 0
progheadsizecluster:	db 0
progheadentsize:	dw 0

gdtr:		dw 0x1F ;size
		dd gdt+(CURRENTSEG * 16) ;offset

gdt:		;first, a null descriptor 0x00
		dq 0
		;code descriptor 0x08, base 0, limit 0xffffffff, type 0x9A
		dw 0xFFFF ;limit 0:15
		;dw KRNLSEG * 16 ;base 0:15
		dw 0x0000
		db 0x00   ;base 16:23
		db 10011010b ;access byte
		db 0xCF   ;flags & limit 16:19
		db 0x00   ;base 24:31
		;data descriptor 0x10, base 0, limit 0xffffffff, type 0x92
		dw 0xFFFF ;limit 0:15
		;dw KRNLSEG * 16 ;base 0:15
		dw 0x0000
		db 0x00   ;base 16:23
		db 10010010b ;access byte
		db 0xCF   ;flags & limit 16:19
		db 0x00   ;base 24:31
		;video descriptor 0x18, base 0x0000b800, limit 0x0000ffff, type 0x92
		dw 0xFFFF ;limit 0:15
		dw 0x8000 ;base 0:15
		db 0x0B   ;base 16:23
		db 10010010b ;access byte
		db 0xCF   ;flags & limit 16:19
		db 0x00   ;base 24:31

