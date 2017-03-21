
OUTPUT = main.o
OBJECTS := jiffy.o irq.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<
	
%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<
