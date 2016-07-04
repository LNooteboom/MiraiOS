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

KERNEL:	kernel.o video.o io.o memory.o irq.o
	${LD} ${LDFLAGS} ld_scripts/kernel

kernel.o:src/kernel.c src/kernel.h src/video.h src/io.h src/param.h src/memory.h
	${CC} ${CFLAG} -c -o kernel.o src/kernel.c

video.o:src/video.asm
	nasm -f elf -o video.o src/video.asm

io.o:	src/io.asm
	nasm -f elf -o io.o src/io.asm

memory.o:src/memory.asm
	nasm -f elf -o memory.o src/memory.asm

irq.o:	src/irq.asm
	nasm -f elf -o irq.o src/irq.asm

clean:
	rm -rf temp
	rm -f out.img
	rm -f BOOT
	rm -f KERNEL
	rm -f *.o
