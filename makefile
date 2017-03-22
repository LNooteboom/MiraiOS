SHELL = /bin/sh

export TARGET = x86_64-elf
export ARCH = x86_64

WARNINGS = "-Wall" "-Wextra"
NOSTDSTUFF = "-ffreestanding" "-nostdlib" "-nostartfiles" "-fno-pie"
export CFLAG = ${WARNINGS} ${NOSTDSTUFF} "-g" "-I$(PWD)/include/" "-I$(PWD)/arch/${ARCH}/include" "-mcmodel=kernel" "-mno-red-zone" "-masm=intel"
export CC = ${TARGET}-gcc
#export CC = gcc

export NASM = nasm
export NASMFLAG = "-f elf64"

export LD = ${TARGET}-ld


OUTPUT = vmmount/out.img
KERNEL = miraiBoot
BOOT = BOOT
IMAGE = out.img
MODULES = kernel arch/${ARCH} drivers mm
OBJ_INIT = init.o


OBJECTS = $(patsubst %, %/main.o, ${MODULES}) ${OBJ_INIT}
INITDIR = arch/${ARCH}/init

all: ${KERNEL} #${BOOT} ${IMAGE}
	#mount ${IMAGE} mnt
	#cp ${KERNEL} mnt
	#cp ${BOOT} mnt
	#umount mnt
	#cp out.img vmmount

${OBJ_INIT}: ${INITDIR}
	$(MAKE) -C $<
	cp ${INITDIR}/main.o $@

${KERNEL}: ${OBJECTS}
	${LD} -z max-page-size=0x1000 -T kernel.ld

%/main.o:
	$(MAKE) -C $(patsubst %/main.o,%,$@)

${BOOT}: boot/stage2.asm
	${NASM} -f bin -o $@ $<

${IMAGE}: boot/bootsector.asm
	${NASM} -f bin -o $@ $<

clean:
	find . -name "*.o" -type f -delete
	rm -f *.img
	rm -f miraiBoot
	rm -f BOOT
