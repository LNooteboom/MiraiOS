
OUTPUT_FORMAT(elf32-i386)

INPUT
(
	/*init.o*/
	arch/x86/main.o
	driver/main.o
	kernel/main.o
	mm/main.o
	/*param/main.o*/
)
STARTUP(init.o)
ENTRY(__init)

OUTPUT(KERNEL)

SECTIONS {
	. = 0xC0100000;
	multiboot : {
		*.o (multiboot);
	}
	.text : {
		init.o (.text);
	}
	.text : {
		*.o (.text);
	}
	.data : {
		*.o (.data);
	}
	.rodata : {
		*.o (.rodata);
	}
	.bss : {
		*.o (.bss);
	}
}
