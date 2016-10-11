
OUTPUT = main.o

all: ${OUTPUT}

${OUTPUT}: main.asm
	nasm -f elf -o $@ $<

