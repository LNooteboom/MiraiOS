NROF_DEFINED_EXCS: equ 21

%define endl 10, 0

extern sprint
extern hexprintln64
extern hexprintln
extern mapIdtEntry
extern testcount
extern lapicBase

extern excPF

global initExceptions:function
global undefinedInterrupt:function
global dummyInterrupt:function

SECTION .text

initExceptions:
    push rbx
    push r12

    xor ebx, ebx
    mov r12, excList
    
    .start:
        cmp ebx, NROF_DEFINED_EXCS
        jae .end
        xor edx, edx
        mov esi, ebx
        mov rdi, [r12]
        call mapIdtEntry
        add r12, 8
        inc ebx
        jmp .start
    .end:

    pop r12
	pop rbx
    ret

exceptionBase:
    push rbp
    mov rbp, rsp

    call sprint

    mov rdi, addressText
    call sprint
    mov rdi, [rbp+8]
    call hexprintln64

    .halt:
        cli
        hlt
        jmp .halt

exceptionBaseWithErrorCode:
    push rbp
    mov rbp, rsp

    call sprint

    mov rdi, addressText
    call sprint
    mov rdi, [rbp+16]
    call hexprintln64

    mov rdi, errorCode
    call sprint
    mov rdi, [rbp+8]
    call hexprintln64

    .halt:
        cli
        hlt
        jmp .halt


excDE:
	mov rdi, DEmsg
	jmp exceptionBase

excDB:
	mov rdi, DBmsg
	jmp exceptionBase
    pop rdi

excNMI:
	iretq

excBP:
	mov rdi, BPmsg
	jmp exceptionBase

excOF:
	mov rdi, OFmsg
	jmp exceptionBase

excBR:
	mov rdi, BRmsg
	jmp exceptionBase

excUD:
	mov rdi, UDmsg
	jmp exceptionBase

excNM:
	mov rdi, NMmsg
	jmp exceptionBase

excDF:
	jmp $
	add esp, 4 ;skip over error code, as it is always zero
	mov rdi, DFmsg 
	jmp exceptionBase

excCSO:
	mov rdi, CSOmsg
	jmp exceptionBase

excTS:
	mov rdi, TSmsg
	jmp exceptionBaseWithErrorCode

excNP:
	mov rdi, NPmsg
	jmp exceptionBaseWithErrorCode

excSS:
	mov rdi, SSmsg
	jmp exceptionBaseWithErrorCode

excGP:
	xchg bx, bx
	mov rdi, GPmsg
	jmp exceptionBaseWithErrorCode

excMF:
	mov rdi, MFmsg
	jmp exceptionBase

excAC:
	mov rdi, ACmsg
	jmp exceptionBaseWithErrorCode

excMC:
	mov rdi, MCmsg
	jmp exceptionBase

excXM:
	mov rdi, XMmsg
	jmp exceptionBase

excVE:
	mov rdi, VEmsg
	jmp exceptionBase

ALIGN 8
undefinedInterrupt:
    iretq

ALIGN 8
dummyInterrupt:
	push rax
	mov rax, [lapicBase]
	mov [rax + 0xB0], dword 0
	pop rax
	iretq

SECTION .rodata

addressText:    db 'At: ', 0
errorCode:      db 'Error code: ', 0


excList:
dq excDE
dq excDB
dq excNMI
dq excBP
dq excOF
dq excBR
dq excUD
dq excNM
dq excDF
dq excCSO
dq excTS
dq excNP
dq excSS
dq excGP
dq excPF
dq undefinedInterrupt
dq excMF
dq excAC
dq excMC
dq excXM
dq excVE

DEmsg:	db 'Division error', endl
DBmsg:	db 'Debug error', endl
BPmsg:	db 'Breakpoint reached', endl
OFmsg:	db 'Overflow', endl
BRmsg:	db 'BOUND Range exceeded', endl
UDmsg:	db 'Invalid opcode detected', endl
NMmsg:	db 'Coprocessor not available', endl
DFmsg:	db 'Double Fault', endl
CSOmsg:	db 'Coprocessor segment overrun', endl
TSmsg:	db 'Invalid TSS', endl
NPmsg:	db 'Segment not present', endl
SSmsg:	db 'Stack fault', endl
GPmsg:	db 'General protection fault', endl
MFmsg:	db 'Coprocessor error', endl
ACmsg:	db 'Alignment check', endl
MCmsg:	db 'Machine check', endl
XMmsg:	db 'SIMD floating point error', endl
VEmsg:	db 'Virtualization exception', endl