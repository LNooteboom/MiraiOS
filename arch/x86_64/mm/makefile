
OUTPUT = main.o
OBJECTS = init.o #map.o 

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<
