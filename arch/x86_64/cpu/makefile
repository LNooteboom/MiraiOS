
OUTPUT = main.o
OBJECTS = lapic.o tssgdt.o pcpu.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

pcpu.o: pcpu.asm smpboot16.bin
	${NASM} ${NASMFLAG} -o $@ $<

smpboot16.bin: smpboot16.asm
	${NASM} -f bin -o $@ $<

mainc.o: main.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<
