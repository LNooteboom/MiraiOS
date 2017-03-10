SUBDIRS = ""
OUTPUT = main.o

OBJECTS = i8253.o
OBJECTS += $(patsubst %, %/main.o, ${SUBDIRS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ ${OBJECTS}

%/main.o: %/makefile
	$(MAKE) -C $(patsubst %/main.o,%,$@)

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<
