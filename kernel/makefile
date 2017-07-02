
OUTPUT = main.o
OBJECTS = mainc.o print.o param.o panic.o
SUBDIRS = 

OBJECTS += $(patsubst %, %/main.o, ${SUBDIRS})

all: ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${LD} -r -o $@ $+

%c.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

%/main.o: %/makefile
	make -C $(patsubst %/main.o,%,$@)