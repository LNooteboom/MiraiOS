
OUTPUT = main.o
OBJECTS = boot.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%.o: %.asm
	${NASM} -g -f elf64 -o $@ $<
