SHELL = /bin/sh
TARGET = i686-elf

CFLAG = "-Wall"
CC = ${TARGET}-gcc

LD = ${TARGET}-ld
LDFLAGS = "-T"
all:	out.img BOOT KERNEL
	mkdir temp
	mount out.img temp/
	cp BOOT temp/
	cp KERNEL temp/
	
	umount temp/
	rm -rf temp/
	cp out.img vmmount/

out.img: src/bootsector.asm
	nasm -f bin -o out.img src/bootsector.asm

BOOT:	src/btst2.asm
	nasm -f bin -o BOOT src/btst2.asm

KERNEL:
	${CC} ${CFLAG} -c -o kernel.o src/kernel.c
	${LD} ${LDFLAGS} ld_scripts/kernel

kernel.o:src/kernel.c
	${CC} ${CFLAG} -c -o kernel.o src/kernel.c

clean:
	rm -rf temp
	rm -f out.img
	rm -f BOOT
	rm -f KERNEL
	rm -f *.o
