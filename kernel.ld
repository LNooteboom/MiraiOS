
OUTPUT_FORMAT(elf64-x86-64)

/*
INPUT
(
	arch/x86_64/main.o
	driver/main.o
	kernel/main.o
	mm/main.o
	param/main.o
)
*/
STARTUP(init.o)
ENTRY(__init)

OUTPUT(miraiBoot)

SECTIONS {
	. = 0x100000;
	multiboot : {
		init.o (multiboot);
	}
	boottext : {
		*.o (boottext);
	}
	bootdata : {
		*.o (bootdata);
	}
	bootbss : {
		*.o (bootbss);
	}
	. += 0xFFFFFFFF00000000;
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
	.bss : {
		*.o (.bss);
		. = ALIGN(4096);
	}
	BSS_END_ADDR = .;
}
