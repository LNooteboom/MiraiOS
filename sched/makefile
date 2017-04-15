
OUTPUT = main.o
OBJECTS = queue.o thread.o readyqueue.o lock.o

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

${OUTPUT}: ${OBJECTS}
	${LD} ${LDFLAG} -o $@ -r $+

clean:
	rm ${OUTPUT} ${OBJECTS}
