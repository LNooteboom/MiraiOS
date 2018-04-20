BITS 64

global setupCRegs:function

setupCRegs:
	mov eax, 1 | (1 << 1) | (1 << 5) | (1 << 31) ;set CR0.PE, CR0.MP, CR0.NE and CR0.PG
	mov cr0, rax

	;set CR4.DE, CR4.PSE, CR4.PAE, CR4.PGE CR4.OSFXSR, CR4.OSXMMEXCPT (and CR4.SMEP if possible)
	xor ecx, ecx
	mov eax, 7
	cpuid

	mov eax, (1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 10)
	test ebx, 7
	jz .noSMEP
		or eax, (1 << 20)
	.noSMEP:
	.cont:
	mov cr4, rax
	ret