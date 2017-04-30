
OUTPUT = main.o
OBJECTS = lapic.o tssgdt.o pcpu.o smpboot.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

smpboot.o: smpboot.asm smpboot16.bin
	${NASM} ${NASMFLAG} -o $@ $<

smpboot16.bin: smpboot16.asm
	${NASM} -f bin -o $@ $<

mainc.o: main.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<
