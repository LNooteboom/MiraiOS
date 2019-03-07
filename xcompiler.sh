#!/bin/bash

PREFIX=$PWD/cross
TARGET_FREESTANDING=x86_64-elf
TARGET_HOSTED=x86_64-miraios

BINUTILS_VERSION=2.32
GCC_VERSION=8.3.0

#sudo apt-get install build-essential genisoimage automake wget nasm bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

mkdir -p cross
cd cross

if [ ! -d binutils-${BINUTILS_VERSION} ]; then
	if [ ! -f binutils-${BINUTILS_VERSION}.tar.xz ]; then
		wget https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
	fi
	echo Untarring binutils...
	tar -xJf binutils-${BINUTILS_VERSION}.tar.xz
	
	cd binutils-${BINUTILS_VERSION}
	echo Patching binutils configs...
	awk '{print $0}/gnu\* \| bsd\**/{print "\t     | miraios* \\"}' config.sub > config2.sub
	mv config2.sub config.sub
	awk '/x86_64-\*-cloudabi\*\)/{print "  x86_64-*-miraios*)\n    targ_defvec=x86_64_elf64_vec\n    targ_selvecs=i386_elf32_vec\n    want64=true\n    ;;"}{print $0}' bfd/config.bfd > bfd/config2.bfd
	mv bfd/config2.bfd bfd/config.bfd
	awk '/i386-\*-linux-\*\)*/{print "  i386-*-miraios*)    fmt=elf ;;"}{print $0}' gas/configure.tgt > gas/configure2.tgt
	mv gas/configure2.tgt gas/configure.tgt
	awk '/x86_64-\*-cloudabi\*\)/{print "x86_64-*-miraios*) targ_emul=elf_x86_64_miraios ;;"}/\*-\*-dragonfly\*\)*/{print "*-*-miraios*)\n  NATIVE_LIB_DIRS=\"/lib /local/lib\"\n  ;;"}{print $0}' ld/configure.tgt > ld/configure2.tgt
	mv ld/configure2.tgt ld/configure.tgt
	printf '. ${srcdir}/emulparams/elf_x86_64.sh\nGENERATE_SHLIB_SCRIPT=yes\nGENERATE_PIE_SCRIPT=yes' > ld/emulparams/elf_x86_64_miraios.sh
	awk '/eelf_x86_64_cloudabi.c:/{print "eelf_x86_64_miraios.c: $(srcdir)/emulparams/elf_x86_64_miraios.sh \\\n  $(ELF_DEPS) $(srcdir)/scripttempl/elf.sc ${GEN_DEPENDS}\n\t${GENSCRIPTS} elf_x86_64_miraios \"$(tdir_elf_x86_64_miraios)\""}{print $0}/ALL_64_EMULATION_SOURCES =*/{print "eelf_x86_64_miraios.c \\"}' ld/Makefile.am > ld/Makefile2.am
	mv ld/Makefile2.am ld/Makefile.am
	cd ld
	autoreconf --force
	automake
	cd ..
	cd ..
fi

if [ ! -d gcc-${GCC_VERSION} ]; then
	if [ ! -f gcc-${GCC_VERSION}.tar.xz ]; then
		wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz
	fi
	echo Untarring gcc...
	tar -xJf gcc-${GCC_VERSION}.tar.xz

	cd gcc-${GCC_VERSION}

	echo Patching gcc configs...
	printf 'MULTILIB_OPTIONS += mno-red-zone\nMULTILIB_DIRNAMES += no-red-zone\n' > gcc/config/i386/t-x86_64-elf
	awk '{print $0 }/x86_64-\*-elf\*\)/{print "\ttmake_file=\"${tmake_file} i386/t-x86_64-elf\""}' gcc/config.gcc > gcc/config2.gcc
	mv gcc/config2.gcc gcc/config.gcc

	awk '{print $0}/-gnu\* \| -bsd\**/{print "\t     | -miraios* \\"}' config.sub > config2.sub
	mv config2.sub config.sub
	awk '/\*-\*-dragonfly\*\)*/{print "*-*-miraios*)\n  gas=yes\n  gnu_ld=yes\n  default_use_cxa_atexit=yes\n  native_system_header_dir=/include\n  ;;"}/x86_64-\*-elf\*\)*/{print "x86_64-*-miraios*)\n    tm_file=\"${tm_file} i386/unix.h i386/att.h dbxelf.h elfos.h glibc-stdint.h i386/i386elf.h i386/x86-64.h miraios.h\"\n    ;;"}{print $0}' gcc/config.gcc > gcc/config2.gcc
	mv gcc/config2.gcc gcc/config.gcc
	cp ../../gccconfig.h gcc/config/miraios.h
	awk '/x86_64-\*-dragonfly\*\)/{print "x86_64-*-miraios*)\n\textra_parts=\"$extra_parts crti.o crtbegin.o crtend.o crtn.o\"\n\ttmake_file=\"$tmake_file i386/t-crtstuff t-crtstuff-pic t-libgcc-pic\"\n\t;;"}{print $0}' libgcc/config.host > libgcc/config2.host
	mv libgcc/config2.host libgcc/config.host
	awk '{print $0}/case*/{print "    *-miraios* | \\"}' fixincludes/mkfixinc.sh > fixincludes/mkfixinc2.sh
	mv fixincludes/mkfixinc.sh2 fixincludes/mkfixinc.sh
	cd ..
fi

if [ ! -f bin/x86_64-elf-as ]; then
	mkdir build-binutils
	cd build-binutils
	../binutils-${BINUTILS_VERSION}/configure --target=$TARGET_FREESTANDING --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make -j 4
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make install
	cd ..
fi

if [ ! -f bin/x86_64-elf-gcc ]; then
	mkdir build-gcc
	cd build-gcc
	../gcc-${GCC_VERSION}/configure --target=$TARGET_FREESTANDING --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make all-gcc -j 4
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make all-target-libgcc -j 4
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make install-gcc
	make install-target-libgcc
	cd ..
fi

mkdir -p ../sysroot/include
mkdir -p ../sysroot/lib
mkdir -p ../sysroot/bin
cp -r ../kernel/include/uapi ../sysroot/include
cp -r ../phlibc/include/* ../sysroot/include

if [ ! -f bin/x86_64-miraios-as ]; then
	rm -rf build-binutils/*
	cd build-binutils
	../binutils-${BINUTILS_VERSION}/configure --target=$TARGET_HOSTED --prefix="$PREFIX" --with-sysroot=$PWD/../../sysroot --disable-werror
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make -j 4
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make install
	cd ..
fi

if [ ! -f bin/x86_64-miraios-gcc ]; then
	rm -rf build-gcc/*
	cd build-gcc
	../gcc-${GCC_VERSION}/configure --target=$TARGET_HOSTED --prefix="$PREFIX" --with-sysroot=$PWD/../../sysroot --enable-languages=c
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make all-gcc -j 4
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make all-target-libgcc -j 4
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	make install-gcc
	make install-target-libgcc
	cd ..
fi

rm -rf build-binutils build-gcc binutils-${BINUTILS_VERSION} gcc-${GCC_VERSION}