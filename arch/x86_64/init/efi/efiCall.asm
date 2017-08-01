global efiCall0:function
global efiCall1:function
global efiCall2:function
global efiCall3:function
global efiCall4:function
global efiCall5:function
global efiCall6:function

SECTION .text

efiCall0:
	sub rsp, 40
	call rdi
	add rsp, 40
	ret

efiCall1:
	sub rsp, 40
	mov rcx, rsi
	call rdi
	add rsp, 40
	ret

efiCall2:
	sub rsp, 40
	;mov rdx, rdx
	mov rcx, rsi
	call rdi
	add rsp, 40
	ret

efiCall3:
	sub rsp, 40
	mov r8, rcx
	;mov rdx, rdx
	mov rcx, rsi
	call rdi
	add rsp, 40
	ret

efiCall4:
	sub rsp, 40
	mov r9, r8
	mov r8, rcx
	;mov rdx, rdx
	mov rcx, rsi
	call rdi
	add rsp, 40
	ret

efiCall5:
	sub rsp, 40
	mov [rsp + 32], r9
	mov r9, r8
	mov r8, rcx
	;mov rdx, rdx
	mov rcx, rsi
	call rdi
	add rsp, 40
	ret

efiCall6:
	sub rsp, 56
	mov rax, [rsp + 64] ;get arg on stack
	mov [rsp + 40], rax
	mov [rsp + 32], r9
	mov r9, r8
	mov r8, rcx
	;mov rdx, rdx
	mov rcx, rsi
	call rdi
	add rsp, 56
	ret