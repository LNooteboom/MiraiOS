SHELL = /bin/sh
export TARGET = i686-elf

export CFLAG = "-Wall"  "-I${PWD}/include/"
export CC = ${TARGET}-gcc

export LD = ${TARGET}-ld

export ARCH = x86

MODULES = mm driver
OBJECTS = $(patsubst %, %/main.o, ${MODULES})
KERNEL = KERNEL

all: ${KERNEL}

test: driver

${KERNEL}: ${OBJECTS}
	#TODO: replace this with linker script
	${LD} -o $@ $+

%/main.o:
	$(MAKE) -C $(patsubst %/main.o,%,$@)

clean:
	find . -name "*.o" -type f -delete
