SHELL = /bin/sh

export TARGET = x86_64-elf

WARNINGS = "-Wall" "-Wextra"
export CFLAG = ${WARNINGS} "-g"  "-I$(PWD)/include/" "-mcmodel=kernel" "-mno-red-zone" "-masm=intel" "-ffreestanding"
export CC = ${TARGET}-gcc

export NASM = nasm
export NASMFLAG = "-f elf64"

export LD = ${TARGET}-ld

export ARCH = x86_64

OUTPUT = vmmount/out.img
KERNEL = miraiBoot
BOOT = BOOT
IMAGE = out.img
MODULES = kernel arch/${ARCH} driver mm
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
