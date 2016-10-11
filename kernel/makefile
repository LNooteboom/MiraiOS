
OUTPUT = main.o
OBJECTS = mainc.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%c.o: %.c
	${CC} ${CFLAG} -c -o $@ $<
