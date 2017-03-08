SUBDIRS = i8253
OUTPUT = main.o
OBJECTS = $(patsubst %, %/main.o, ${SUBDIRS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ ${OBJECTS}

%/main.o: %/makefile
	$(MAKE) -C $(patsubst %/main.o,%,$@)
