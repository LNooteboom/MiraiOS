BITS 64

ORG 0x1000

;ud2
xchg bx, bx
xor eax, eax
syscall
;int 3
jmp $