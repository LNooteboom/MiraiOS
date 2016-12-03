SHELL = /bin/sh

export PWD = /home/luke/mirai
export TARGET = amd64-elf

WARNINGS = "-Wall" "-Wextra"
export CFLAG = ${WARNINGS}  "-I${PWD}/include/"
export CC = ${TARGET}-gcc

export NASM = nasm

export LD = ${TARGET}-ld

export ARCH = x86_64

OUTPUT = vmmount/out.img
KERNEL = miraiBoot
BOOT = BOOT
IMAGE = out.img
MODULES = #arch/${ARCH} mm driver kernel param
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
	${LD} -T kernel.ld

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
