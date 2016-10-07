export NASM = nasm
export NASMFLAG = ""

OUTPUT = arch.o
OBJECTS = irq.o pic.o pit.o

test: irq.o

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} -f elf -o $@ $<
