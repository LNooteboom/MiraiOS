
BITS 16

	jmp short start

startedUpFlag:	dd 0

start:
	mov ax, cs
	mov dl, 1
	mov ds, ax
	mov es, ax
	mov [startedUpFlag], dl
	jmp $