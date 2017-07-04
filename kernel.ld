
OUTPUT_FORMAT(elf64-x86-64)


INPUT
(
	mm/main.o
	drivers/main.o
	arch/x86_64/main.o
	kernel/main.o
	sched/main.o
)

STARTUP(init.o)
ENTRY(__init)

OUTPUT(miraiBoot)

SECTIONS {
	VMEM_OFFSET = 0xFFFFFFFF80000000;
	PAGE_SIZE = 0x1000;
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
		*.o (.rodata*);
	}

	.moduleInits : {
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
	.bss : {
		*.o (.bss);
		. = ALIGN(4096);
	}
	BSS_END_ADDR = .;
}
