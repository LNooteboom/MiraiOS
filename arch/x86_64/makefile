
OUTPUT = main.o
SUBDIRS = irq mm#timer pic
OBJECTS = spinlock.o


OBJECTS += $(patsubst %, %/main.o, ${SUBDIRS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<

%/main.o: %/makefile
	make -C $(patsubst %/main.o,%,$@)
