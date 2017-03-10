
OUTPUT = main.o
OBJECTS = idt.o mainc.o exception.o hwirq.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

mainc.o: main.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<
