global excPF:function

extern puts
extern hexprintln
extern hexprintln64
extern cprint

extern panic

extern mmGetEntry
extern allocPhysPage
extern allocCleanPhysPage
extern allocLargePhysPage
extern allocLargeCleanPhysPage

%define endl 10, 0
%define NROF_PAGE_LEVELS 4

SECTION .text

excPF:
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push rbx
	push r12

	mov rax, 0xffffffff80000000
	cmp [rsp + 0x60], rax
	jae .noswapgs
		swapgs
		or [rsp + 0x80], dword 3
	.noswapgs:

    ;test if caused by not present page
    mov eax, [rsp + 0x58]
    test eax, 1
    jnz .L0
        mov bl, NROF_PAGE_LEVELS - 1
        .L1:
            mov rdi, cr2
            movzx esi, bl
            call mmGetEntry
            ;rax contains pointer to pte
            mov r12, rax
            mov rax, [rax]
            test al, 0x01 ;present flag
            jnz .L2
                test eax, (1 << 9) ;inuse flag
                jz .L0 ;give error if not in use
                cmp bl, 0
                jne .L3
                    test eax, (1 << 10) ;clean flag
                    jz .L4
						call allocCleanPhysPage
                        jmp .L7
                    .L4:
                        call allocPhysPage
                        jmp .L7
                .L3:
                cmp bl, 1
                jne .L5
                    test ax, (1 << 10) ;clean flag
                    jz .L6
                        ;call allocLargeCleanPhysPage
                        jmp .L7
                    .L6:
                        call allocLargePhysPage
                        jmp .L7
                .L5: ;else
                    mov rdi, invAllocMsg
                    call puts
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
			call puts
			jmp $
        .L8:
        xor rdi, rdi
        mov cr2, rdi

		mov rax, 0xffffffff80000000
		cmp [rsp + 0x60], rax
		jae .noswapgs2
			swapgs
		.noswapgs2:
        
        pop r12
		pop rbx
		pop r11
		pop r10
		pop r9
		pop r8
		pop rdi
		pop rsi
		pop rdx
		pop rcx
		pop rax
        add rsp, 8 ;jump over error code
        iretq
    .L0:
	add rsp, 0x58

    ;print error message
    mov rdi, PFmsg
    call puts

    ;print return addr
    mov rdi, addressText ;"At: "
    call puts
    mov rdi, [rsp+8]
    call hexprintln64

    ;print "attempted to access"
    mov rdi, PFAddr
    call puts
    mov rdi, cr2
    call hexprintln64

    ;print error code
    mov rdi, errorCode
    call puts
    mov edi, [rsp]
    call hexprintln64
    test [rsp], byte 0x01
    jz .L9
        mov rdi, PFPresent
        call puts
    .L9:
    test [rsp], byte 0x02
    jz .L10
        mov rdi, PFRW
        call puts
    .L10:
    test [rsp], byte 0x04
    jz .L11
        mov rdi, PFUS
        call puts
    .L11:
    test [rsp], byte 0x08
    jz .L12
        mov rdi, PFRSV
        call puts
		mov rdi, cr2
		mov rsi, 0
		call mmGetEntry
		mov rdi, [rax]
		call hexprintln64
    .L12:
    test [rsp], byte 0x10
    jz .L13
        mov rdi, PFID
        call puts
    .L13:

	mov rdi, PFmsg
	call panic

    jmp $

SECTION .rodata
PFmsg:	db 'Page fault', endl
addressText:    db 'At: ', 0
errorCode:      db 'Error code: ', 0

PFAddr:         db 'Attempted to access ', 0
PFPresent:      db 'Page present', endl
PFRW:           db 'Write', endl
PFUS:           db 'Came from userspace', endl
PFRSV:          db '1 found in reserved field', endl
PFID:           db 'Caused by instruction fetch', endl

invAllocMsg:    db 'Invalid page alloc', endl

weirdPF:		db 'Weird Page fault occurred!', endl