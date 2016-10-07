SHELL = /bin/sh
export TARGET = i686-elf

#export LD_LIBRARY_PATH = ${PWD}/include/

export CFLAG = "-Wall"  "-I${PWD}/include/"
export CC = ${TARGET}-gcc

export LD = ${TARGET}-ld
export LDFLAGS = ""

obj_all = mm/mm.o


mm.o: 
	cd mm && $(MAKE)

%.o: %.c
	${CC} ${CFLAG} -c -o $@ $<

clean:
	cd mm && $(MAKE) clean

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

