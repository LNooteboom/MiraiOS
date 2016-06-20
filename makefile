SHELL = /bin/sh

CFLAG = "-Wall"
CC = ${TARGET}-gcc

LD = ${TARGET}-ld
LDFLAGS = "-T"
all:	out.img BOOT
	mkdir temp
	mount out.img temp/
	cp BOOT temp/
	
	umount temp/
	rm -rf temp/
	cp out.img vmmount/

out.img: src/bootsector.asm
	nasm -f bin -o out.img src/bootsector.asm

BOOT:	btst2.o kernel.o ld_scripts/kernel
	${LD} ${LDFLAGS} ld_scripts/kernel
	
btst2.o: src/btst2.asm
	nasm -f elf -o btst2.o src/btst2.asm

kernel.o:src/kernel.c
	${CC} ${CFLAG} -c -o kernel.o src/kernel.c

clean:
	rm -rf temp
	rm -f out.img
	rm -f BOOT
	rm -f *.o
