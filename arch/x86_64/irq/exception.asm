BITS 64
DEFAULT REL

NROF_DEFINED_EXCS: equ 21

%define endl 10, 0

extern sprint
extern hexprintln64
extern hexprintln
extern routeInterrupt
extern testcount

global initExceptions:function

SECTION .text

initExceptions:
    push rbp
    mov rbp, rsp

    push rbx
    push r12
    xor bl, bl
    mov r12, excList
    
    .start:
        cmp bl, NROF_DEFINED_EXCS
        jae .end
        mov dl, 1
        mov si, bx
        mov rdi, [r12]
        call routeInterrupt
        add r12, 8
        inc bl
        jmp .start
    .end:

    mov r12, [rbp-16]
    mov rbx, [rbp-8]
    leave
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
	iret

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
	mov rdi, GPmsg
	jmp exceptionBaseWithErrorCode

excPF:
    ;print error message
    mov rdi, PFmsg
    call sprint

    ;print return addr
    mov rdi, addressText ;"At: "
    call sprint
    mov rdi, [rsp+8]
    call hexprintln64

    ;print "attempted to access"
    mov rdi, PFAddr
    call sprint
    mov rdi, cr2
    call hexprintln64

    ;print error code
    mov rdi, errorCode
    call sprint
    mov edi, [rsp]
    call hexprintln

    test [rsp], byte 0x01
    jz .L0
        mov rdi, PFPresent
        call sprint
    .L0:
    test [rsp], byte 0x02
    jz .L1
        mov rdi, PFRW
        call sprint
    .L1:
    test [rsp], byte 0x04
    jz .L2
        mov rdi, PFUS
        call sprint
    .L2:
    test [rsp], byte 0x08
    jz .L3
        mov rdi, PFRSV
        call sprint
    .L3:
    test [rsp], byte 0x10
    jz .L4
        mov rdi, PFID
        call sprint
    .L4:

    jmp $

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

undefinedInterrupt:
    iret


SECTION .rodata

addressText:    db 'At: ', 0
PFAddr:         db 'Attempted to access ', 0
errorCode:      db 'Error code: ', 0

PFPresent:      db 'Page not present', endl
PFRW:           db 'Write to read-only page', endl
PFUS:           db 'Priviledge too low', endl
PFRSV:          db '1 found in reserved field', endl
PFID:           db 'Caused by instruction fetch', endl


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
PFmsg:	db 'Page fault', endl
MFmsg:	db 'Coprocessor error', endl
ACmsg:	db 'Alignment check', endl
MCmsg:	db 'Machine check', endl
XMmsg:	db 'SIMD floating point error', endl
VEmsg:	db 'Virtualization exception', endl