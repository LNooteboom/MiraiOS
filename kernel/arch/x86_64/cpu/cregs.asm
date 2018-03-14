BITS 64

global setupCRegs:function

setupCRegs:
	mov eax, 1 | (1 << 1) | (1 << 5) | (1 << 31) ;set CR0.PE, CR0.MP, CR0.NE and CR0.PG
	mov cr0, rax

	;set CR4.DE, CR4.PSE, CR4.PAE, CR4.PGE (and CR4.SMEP if possible) TODO SSE
	xor ecx, ecx
	mov eax, 7
	cpuid
	test ebx, 7
	jz .noSMEP
		mov eax, (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 20)
	.noSMEP:
		mov eax, (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7)
	.cont:
	mov cr4, rax
	ret