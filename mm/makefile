
OBJECTS = physpaging.o paging.o

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

mm.o: ${OBJECTS}
	${LD} ${LDFLAG} -R -o $@ $+
