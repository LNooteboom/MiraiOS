
OUTPUT_FORMAT(elf64-x86-64)


INPUT
(
	mm/main.o
	drivers/main.o
	arch/x86_64/main.o
	kernel/main.o
)

STARTUP(init.o)
ENTRY(__init)

OUTPUT(miraiBoot)

SECTIONS {
	VMEM_OFFSET = 0xFFFFFFFF80000000;
	. = 0x1000000;
	KERNEL_START_ADDR = . + VMEM_OFFSET;
	multiboot : {
		init.o (multiboot);
	}
	boottext : {
		*.o (boottext);
	}
	bootdata : {
		*.o (bootdata);
	}
	. += VMEM_OFFSET;
	.text : {
		*.o (.text);
	}
	TEXT_END_ADDR = .;
	.data : {
		*.o (.data);
	}
	.rodata : {
		*.o (.rodata);
	}
	DATA_END_ADDR = .;
	.bss : {
		*.o (.bss);
		. = ALIGN(4096);
	}
	BSS_END_ADDR = .;
}
