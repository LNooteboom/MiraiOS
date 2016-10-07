
OUTPUT = mm.o
OBJECTS = physpaging.o paging.o

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

${OUTPUT}: ${OBJECTS}
	${LD} ${LDFLAG} -o $@ -r $+

clean:
	rm ${OUTPUT} ${OBJECTS}
