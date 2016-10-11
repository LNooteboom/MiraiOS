
OUTPUT_FORMAT(elf32-i386)

INPUT
(
	*.o
)
STARTUP(kernel.o)

OUTPUT(KERNEL)
ENTRY(kmain)

SECTIONS {
	. = 0xC0100000;
/*	.text : {
		irq.o (.text);
	}*/
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
