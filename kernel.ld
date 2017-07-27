
OUTPUT_FORMAT(elf64-x86-64)

/*STARTUP(init.o)*/
ENTRY(__init)

OUTPUT(miraiBoot)

SECTIONS {
	VMEM_OFFSET = 0xFFFFFFFF80000000;
	PAGE_SIZE = 0x1000;

	KERNEL_START_ADDR = . + 0x1000000 + VMEM_OFFSET;
	.setup : ALIGN(0x1000) {
		*.o (.setup);
		*.o (.setupdata);
	}
	. += 0x1000000 + VMEM_OFFSET;
	.text : ALIGN(0x1000) {
		*.o (.text);
	}
	TEXT_END_ADDR = .;
	.data : ALIGN(0x1000) {
		*.o (.data);
	}
	.rodata : ALIGN(0x1000) {
		*.o (.rodata*);
	}

	.moduleInits : ALIGN(0x1000) {
		MODULE_INITS_0_START = .;
		*.o (.moduleInits0)
		MODULE_INITS_1_START = .;
		*.o (.moduleInits1)
		MODULE_INITS_2_START = .;
		*.o (.moduleInits2)
		MODULE_INITS_3_START = .;
		*.o (.moduleInits3)
		MODULE_INITS_END = .;
	}

	DATA_END_ADDR = .;
	.bss : ALIGN(0x1000) {
		*.o (.bss);
		. = ALIGN(4096);
	}
	BSS_END_ADDR = .;
}
