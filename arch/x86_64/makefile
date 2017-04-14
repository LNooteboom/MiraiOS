
OUTPUT = main.o
SUBDIRS = irq mm acpi cpu sched
OBJECTS = spinlock.o mainc.o


OBJECTS += $(patsubst %, %/main.o, ${SUBDIRS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%c.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<

%/main.o: %/makefile
	make -C $(patsubst %/main.o,%,$@)
