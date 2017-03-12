OUTPUT = main.o
OBJECTS = lapicc.o lapic.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

mainc.o: main.c
	${CC} ${CFLAG} -c -o $@ $<

%c.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<