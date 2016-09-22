SHELL = /bin/sh
TARGET = i686-elf

CFLAG = "-Wall -c"
CC = ${TARGET}-gcc

LD = ${TARGET}-ld
LDFLAGS = "-T"

OBJECTS = "*.o"

all:	out.img BOOT KERNEL
	mkdir temp
	mount out.img temp/
	cp BOOT temp/
	cp KERNEL temp/
	
	umount temp/
	rm -rf temp/
	cp out.img vmmount/

out.img: boot/bootsector.asm
	nasm -f bin -o $@ $+

BOOT: boot/stage2.asm
	nasm -f bin -o $@ $+

KERNEL: OBJECTS
	${LD} -T ld_scripts/kernel.lds

kmain.o: main/kmain.c
	${CC} ${CFLAG} -o $@ $+

param.o: param/param.c
	${CC} ${CFLAG} -o $@ $+

memory.o: memory_c.o memory_asm.o
	ld -o $@ $+

memory_c.o: memory/init.c memory/memdetect.c memory/paging.c memory/alloc.c memory/dealloc.c
	${CC} ${CFLAG} -o $@ $+

memory_asm.o: memory/init.asm
	nasm -f elf -o $@ $+
