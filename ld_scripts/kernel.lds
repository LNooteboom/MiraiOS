
OUTPUT_FORMAT(elf32-i386)

INPUT
(
	video.o
	io.o
	memory.o
	irq.o
	tty.o
	ps2.o
	pit.o
	keyb.o
	param.o
)
STARTUP(kernel.o)

OUTPUT(KERNEL)
ENTRY(kmain)

SECTIONS {
	. = 0xC0000000;
	.text : {
		irq.o (.text);
	}
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
