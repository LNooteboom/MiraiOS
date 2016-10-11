
MAINOBJ = main.o
OBJECTS = mainc.o mainasm.o

all: ${MAINOBJ}

${MAINOBJ}: ${OBJECTS}
	${LD} -r -o $@ $+

%c.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%asm.o: %.asm
	nasm -f elf -o $@ $<

clean:
	rm *.o

