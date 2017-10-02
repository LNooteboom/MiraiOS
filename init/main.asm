BITS 64

ORG 0x1000

;ud2
nop

xchg bx, bx
mov eax, 32
mov edi, 2
mov esi, 0x1000
syscall

xchg bx, bx
mov al, [gs:0]
jmp $