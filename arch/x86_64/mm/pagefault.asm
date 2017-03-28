BITS 64

global excPF:function

extern sprint
extern hexprintln
extern hexprintln64
extern cprint

extern mmGetEntry
extern allocPhysPage
extern allocCleanPhysPage
extern allocLargePhysPage
extern allocLargeCleanPhysPage

%define endl 10, 0
%define NROF_PAGE_LEVELS 4

SECTION .text

excPF:
    push rbp
    pushfq
    mov rbp, rsp
    sub rsp, 0x58
    mov [rbp-0x08], rax
    mov [rbp-0x10], rcx
    mov [rbp-0x18], rdx
    mov [rbp-0x20], rdi
    mov [rbp-0x28], rsi
    mov [rbp-0x30], r8
    mov [rbp-0x38], r9
    mov [rbp-0x40], r10
    mov [rbp-0x48], r11
    mov [rbp-0x50], rbx
    mov [rbp-0x58], r12
	cld

    ;test if caused by not present page
    mov eax, [rbp+16]
    test eax, 1
    jnz .L0
        mov bl, NROF_PAGE_LEVELS - 1
        .L1:
            mov rdi, cr2
            movzx si, bl
            call mmGetEntry
            ;rax contains pointer to pte
            mov r12, rax
            mov rax, [rax]
            test al, 0x01 ;present flag
            jnz .L2
                test ax, (1 << 9) ;inuse flag
                jz .L0 ;give error if not in use
                cmp bl, 0
                jne .L3
                    test ax, (1 << 10) ;clean flag
                    jnz .L4
                        call allocCleanPhysPage
                        jmp .L7
                    .L4:
                        call allocPhysPage
                        jmp .L7
                .L3:
                cmp bl, 1
                jne .L5
                    test ax, (1 << 10) ;clean flag
                    jnz .L6
                        call allocLargeCleanPhysPage
                        jmp .L7
                    .L6:
                        call allocLargePhysPage
                        jmp .L7
                .L5: ;else
                    mov rdi, invAllocMsg
                    call sprint
                    jmp $
                .L7:
                mov rdx, 0xFFF0000000000FFF
                or al, 1 ;set present bit
                and [r12], rdx  ;clear address field
                or [r12], rax ;OR it with new address + present bit
                jmp .L8
            .L2:
            sub bl, 1
            jns .L1
            ;page already alloced
			mov rdi, weirdPF
			call sprint
			jmp $
        .L8:
        xor rdi, rdi
        mov cr2, rdi
        
        mov rax, [rbp-0x08]
        mov rcx, [rbp-0x10]
        mov rdx, [rbp-0x18]
        mov rdi, [rbp-0x20]
        mov rsi, [rbp-0x28]
        mov r8,  [rbp-0x30]
        mov r9,  [rbp-0x38]
        mov r10, [rbp-0x40]
        mov r11, [rbp-0x48]
        mov rbx, [rbp-0x50]
        mov r12, [rbp-0x58]
        mov rsp, rbp
        popfq
        pop rbp
        add rsp, 8 ;jump over error code
        iretq
    .L0:
	add rbp, 16
	mov rsp, rbp
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
    jz .L9
        mov rdi, PFPresent
        call sprint
    .L9:
    test [rsp], byte 0x02
    jz .L10
        mov rdi, PFRW
        call sprint
    .L10:
    test [rsp], byte 0x04
    jz .L11
        mov rdi, PFUS
        call sprint
    .L11:
    test [rsp], byte 0x08
    jz .L12
        mov rdi, PFRSV
        call sprint
		mov rdi, cr2
		mov rsi, 0
		call mmGetEntry
		mov rdi, [rax]
		call hexprintln64
    .L12:
    test [rsp], byte 0x10
    jz .L13
        mov rdi, PFID
        call sprint
    .L13:

    jmp $

SECTION .rodata
PFmsg:	db 'Page fault', endl
addressText:    db 'At: ', 0
errorCode:      db 'Error code: ', 0

PFAddr:         db 'Attempted to access ', 0
PFPresent:      db 'Page present', endl
PFRW:           db 'Write', endl
PFUS:           db 'Priviledge too low', endl
PFRSV:          db '1 found in reserved field', endl
PFID:           db 'Caused by instruction fetch', endl

invAllocMsg:    db 'Invalid page alloc', endl

weirdPF:		db 'Weird Page fault occurred!', endl