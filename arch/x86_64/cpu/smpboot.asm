global smpbootStart:function

global smpboot16start:data
global smpboot16end:data

extern physLapicBase
extern lapicBase
extern cpuInfoSize
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
		cmp [rax + 16], edx ;compare this cpu's apic id
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
	mov rsp, [rax + 0x08]
	mov rbp, [rax + 0x08]

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

	xor edi, edi
	call setCurrentThread
	;increment nrof active cpus
	lock inc dword [nrofActiveCPUs]
	;sti

	.loadThread:
	call readyQueuePop
	test rax, rax
	jnz .threadLoaded
		sti
		hlt
		cli
		jmp .loadThread
	.threadLoaded:
	mov r15, rax ;15 now contains current thread
	lea rdi, [rax + 0x14]
	call acquireSpinlock
	mov rdi, r15
	mov [r15 + 0x10], dword 1 ;set state to RUNNING
	call setCurrentThread

	;get new rsp
	mov rsp, [r15]
	lea rdi, [r15 + 0x14]
	call releaseSpinlock

	;restore all registers
	mov rax, [rsp + 0x70]
	mov rcx, [rsp + 0x68]
	mov rdx, [rsp + 0x60]
	mov rdi, [rsp + 0x58]
	mov rsi, [rsp + 0x50]
	mov r8,  [rsp + 0x48]
	mov r9,  [rsp + 0x40]
	mov r10, [rsp + 0x38]
	mov r11, [rsp + 0x30]
	mov rbx, [rsp + 0x28]
	mov rbp, [rsp + 0x20]
	mov r12, [rsp + 0x18]
	mov r13, [rsp + 0x10]
	mov r14, [rsp + 0x08]
	mov r15, [rsp]
	add rsp, 0x78
	iretq ;will likely set irq flag

	.error:
	cli
	hlt
	jmp .error

SECTION .rodata

smpboot16start:
INCBIN "smpboot16.bin"
smpboot16end: