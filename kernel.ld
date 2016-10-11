
OUTPUT_FORMAT(elf32-i386)

INPUT
(
	init.o
	arch.o
	driver.o
	kernel.o
	mm.o
	param.o
)
STARTUP(init.o)

OUTPUT(KERNEL)
ENTRY(__init)

SECTIONS {
	. = 0x00100000;
	multiboot : {
		init.o (multiboot);
	}
	boottext : {
		init.o (boottext);
	}
	bootdata : {
		init.o (bootdata);
	}
	bootbss : {
		init.o (bootbss);
	}

	. = 0xC0100000;
	.text : {
		*.o (.text);
	}
	.data : {
		*.o (.data);
	}
	.bss : {
		*.o (.bss);
	}
}
