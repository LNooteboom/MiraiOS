
OUTPUT = main.o
OBJECTS = map.o init.o

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<
