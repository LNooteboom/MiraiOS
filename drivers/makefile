
DRIVERS = vga# ps2 ps2keyboard
OUTPUT = main.o
OBJECTS = $(patsubst %, %/main.o, ${DRIVERS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ ${OBJECTS}

%/main.o: %/makefile
	$(MAKE) -C $(patsubst %/main.o,%,$@)

