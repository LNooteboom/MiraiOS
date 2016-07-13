SHELL = /bin/sh
TARGET = i686-elf

CFLAG = "-Wall"
CC = ${TARGET}-gcc

LD = ${TARGET}-ld
LDFLAGS = "-T"

KRNLHEADERS = src/kernel.h src/video.h src/io.h src/irq.h src/tty.h src/memory.h src/param.h src/ps2.h
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

KERNEL:	kernel.o keyb.o pit.o ps2.o video.o io.o memory.o irq.o tty.o ${KRNLHEADERS}
	${LD} ${LDFLAGS} ld_scripts/kernel

kernel.o:src/kernel.c src/kernel.h
	${CC} ${CFLAG} -c -o kernel.o src/kernel.c

video.o:src/video.asm
	nasm -f elf -o video.o src/video.asm

io.o:	src/io.asm
	nasm -f elf -o io.o src/io.asm

memory.o:src/memory.asm
	nasm -f elf -o memory.o src/memory.asm

irq.o:	src/irq.asm
	nasm -f elf -o irq.o src/irq.asm

tty.o:	src/tty.c src/tty.h
	${CC} ${CFLAG} -c -o tty.o src/tty.c

ps2.o:	ps2c.o ps2asm.o
	${LD} -r -o ps2.o ps2c.o ps2asm.o

ps2c.o:	src/ps2.c
	${CC} ${CFLAG} -c -o ps2c.o src/ps2.c

ps2asm.o:src/ps2.asm
	nasm -f elf -o ps2asm.o src/ps2.asm

pit.o:	src/pit.asm
	nasm -f elf -o pit.o src/pit.asm

keyb.o:	src/keyb.c src/keyb.h
	${CC} ${CFLAG} -c -o keyb.o src/keyb.c

clean:
	rm -rf temp
	rm -f out.img
	rm -f BOOT
	rm -f KERNEL
	rm -f *.o
