
DRIVERS = vga timer
OUTPUT = main.o
OBJECTS := $(patsubst %, %/main.o, ${DRIVERS}) input/ps2/i8042.o input/ps2/keyboard.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ ${OBJECTS}

%/main.o: %/makefile
	$(MAKE) -C $(patsubst %/main.o,%,$@)

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.asm
	${NASM} ${NASMFLAG} -o $@ $<