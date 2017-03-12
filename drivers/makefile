
DRIVERS = vga timer
OUTPUT = main.o
OBJECTS := $(patsubst %, %/main.o, ${DRIVERS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ ${OBJECTS}

%/main.o: %/makefile
	echo a
	echo $@
	$(MAKE) -C $(patsubst %/main.o,%,$@)
	echo b
