
global lapicEnable:function
global setTSCAUX:function
global getCPU:function

extern useTSCP

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

setTSCAUX: ;(uint32_t value) returns bool rdtscpPresent
	mov eax, 0x80000001
	cpuid
	mov esi, edx
	shr esi, 27
	and esi, 1
	jz .end

	xor edx, edx
	mov eax, edi
	mov ecx, 0xC0000103
	wrmsr

	.end:
	mov eax, esi
	;mov edi, esi
	;call hexprintln
	ret

getCPU: ;(void) returns uint32_t index
	cmp [useTSCP], byte 0
	je .noTSCP
		rdtscp
		mov eax, ecx
		jmp .end
	.noTSCP:
		;use gdt instead
		mov edx, 0x38
		lsl eax, edx
	.end:
	ret