SHELL = /bin/sh
export TARGET = i686-elf

export CFLAG = "-Wall"  "-I${PWD}/include/"
export CC = ${TARGET}-gcc

export LD = ${TARGET}-ld

export ARCH = x86

KERNEL = KERNEL
MODULES = mm driver
OBJ_INIT = init.o


OBJECTS = $(patsubst %, %/main.o, ${MODULES}) + ${OBJ_INIT}
INITDIR = arch/${ARCH}/init

all: ${KERNEL}

${OBJ_INIT}: ${INITDIR}/makefile
	$(MAKE) -C $<
	cp ${INITDIR}/main.o $@

${KERNEL}: ${OBJECTS}
	${LD} -T kernel.ld

%/main.o:
	$(MAKE) -C $(patsubst %/main.o,%,$@)

clean:
	find . -name "*.o" -type f -delete
