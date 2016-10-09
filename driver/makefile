
DRIVERS = ps2 ps2keyboard vga tty
OUTPUT = drivers.o
OBJECTS = $(patsubst %, %/main.o, ${DRIVERS})


all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ ${OBJECTS}

%/main.o: %/makefile
	cd % && $(MAKE)
