SHELL = /bin/sh
TARGET = i686-elf

export LD_LIBRARY_PATH = ${PWD}/include/

CFLAG = "-Wall -std=gnu99"
CC = ${TARGET}-gcc

LD = ${TARGET}-ld
LDFLAGS = "-T"

obj_all = ${obj_mm}

obj_mm = paging.o physpaging.o

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

clean:
	rm *.o

test: obj_all

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

KERNEL: ${OBJECTS}
	${LD} -T ld_scripts/kernel.lds

