global smpbootStart:function

global smpboot16start:data
global smpboot16end:data

extern physLapicBase
extern lapicBase
extern cpuInfoSize
extern cpuStartedUp

extern cpuInfos
extern nrofCPUs
extern nrofActiveCPUs
extern initStackEnd
extern idtr
extern tssGdtInit
extern acquireSpinlock
extern releaseSpinlock
extern setCurrentThread
extern readyQueuePop
extern perCpuTimer
extern lapicEnableTimer

extern syscallInit
extern nextThread

SECTION .text

smpbootStart:
	;jump to high address
	mov rax, .start
	jmp rax

	.start:
	;set segment regs
	mov eax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	;get APIC ID
	mov eax, [physLapicBase]
	mov edx, [physLapicBase + 4]
	mov ecx, 0x1B
	or eax, (1 << 11)
	wrmsr ;enable local apic

	mov rsi, [lapicBase]
	;get APIC ID
	mov edx, [rsi + 0x20]
	shr edx, 24

	;now get cpuinfo for this cpu
	mov rax, [cpuInfos]
	xor ecx, ecx
	mov rdi, [cpuInfoSize]
	.loop:
		cmp [rax + 24], edx ;compare this cpu's apic id
		je .end
		
		inc ecx
		add rax, rdi
		cmp ecx, [nrofCPUs]
		jne .loop
		nop
		jmp .error
	.end:
	;pointer to cpuinfo is in rax, index+1 in ecx
	dec ecx

	;get exception stack
	mov rsp, [rax + 0x10]
	mov rbp, [rax + 0x10]

	;load idt
	lidt [idtr]
	;rsi still contains lapic base
	mov [rsi + 0xF0], dword 0x1FF ;set spurious interrupt register to 0xFF, set ASE
	;rax still contains pointer to cpuinfo
	mov r15, rax
	mov rdi, rax
	call tssGdtInit

	mov rdx, r15
	mov eax, r15d
	mov ecx, 0xC0000101
	shr rdx, 32
	wrmsr ;set GS.base to CPUInfo

	;enable APIC timer
	cmp [perCpuTimer], dword 0
	je .noApicTimer
		mov edi, 0xC3
		call lapicEnableTimer
	.noApicTimer:

	;init syscall registers
	call syscallInit

	xor edi, edi
	call setCurrentThread
	;increment nrof active cpus
	lock inc dword [nrofActiveCPUs]
	;set started up flag
	;mov [cpuStartedUp], dword 1
	mov [r15 + 0x20], dword 1

	xor r15, r15
	jmp nextThread

	.error:
	cli
	hlt
	jmp .error

SECTION .rodata

smpboot16start:
INCBIN "arch/x86_64/cpu/smpboot16.bin"
smpboot16end: