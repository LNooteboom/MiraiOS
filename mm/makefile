
OUTPUT = main.o
OBJECTS = init.o physpaging.o heap.o

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

${OUTPUT}: ${OBJECTS}
	${LD} ${LDFLAG} -o $@ -r $+

clean:
	rm ${OUTPUT} ${OBJECTS}
