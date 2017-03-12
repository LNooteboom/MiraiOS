
global lapicEnable:function

SECTION .text

lapicEnable: ;(uintptr_t *baseAddr, bool *isBSP) returns void
	mov ecx, 0x1B ;apic base address register
	rdmsr
	or eax, (1 << 11) ;set APIC enable bit
	wrmsr
	and eax, 0xFFFFF000
	mov [rdi+4], edx
	mov [rdi], eax
	ret
