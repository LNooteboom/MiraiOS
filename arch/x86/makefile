export NASM = nasm

OUTPUT = arch.o
SUBDIRS = irq #timer mmu pic
OBJECTS = io.o


OBJECTS += $(patsubst %, %/main.o, ${SUBDIRS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} -f elf -o $@ $<

%/main.o: %/makefile
	make -C $(patsubst %/main.o,%,$@)
