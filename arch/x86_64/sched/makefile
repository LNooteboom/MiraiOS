
OUTPUT = main.o
OBJECTS = threadc.o thread.o spinlock.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%c.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

