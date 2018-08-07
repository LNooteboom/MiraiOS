#!/bin/bash

buildUserspace () {
	if [ -f "$1/.buildtime" ] && [ "$SYSROOT/lib/libc.a" -nt "$1/.buildtime" ]; then
		make -C $1 clean
		echo clean
	fi
	make -C $1
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi

	touch $1/.buildtime
}

#config options
export TARGET_KERNEL=x86_64-elf
export TARGET_USER=x86_64-miraios

export DEPDIR=.d
export FLAG_DEP="-MT \$@ -MMD -MP -MF $DEPDIR/\$*.Td"
export CFLAG_USER="-Wall -Wextra -g -O2 -std=gnu99"

BUILDDIR=build;

export INITRDDIR=$PWD/initrd;
export SYSROOT=$PWD/sysroot
export PREFIX=$SYSROOT

KERNELDIR=kernel;
KERNELNAME=miraiBoot;

EFI=false;

mkdir -p $BUILDDIR
mkdir -p $INITRDDIR

#build kernel
make -j 4 -C $KERNELDIR
OUT=$?
if [ ! $OUT -eq 0 ]; then
	exit $OUT
fi
if [ -f "$KERNELDIR/$KERNELNAME.efi" ] && [ ! "$KERNELDIR/$KERNELNAME.efi" -ot "$KERNELDIR/$KERNELNAME" ]; then
	#efi
	echo "Building for EFI"
	EFI=true;
	BUILDDIR=$BUILDDIR/EFI/BOOT;

	mkdir -p $BUILDDIR
	cp "$KERNELDIR/$KERNELNAME.efi" $BUILDDIR/BOOTx64.efi
else
	#bios
	echo "Building for legacy BIOS"
	EFI=false;
	BUILDDIR=$BUILDDIR/boot;

	mkdir -p $BUILDDIR
	cp "$KERNELDIR/$KERNELNAME" $BUILDDIR
fi

mkdir -p $SYSROOT/include
mkdir -p $SYSROOT/lib
mkdir -p $SYSROOT/bin
cp -r $KERNELDIR/include/uapi $SYSROOT/include

#build libc
make -C phlibc
OUT=$?
if [ ! $OUT -eq 0 ]; then
	exit $OUT
fi
cp -r phlibc/include/* $SYSROOT/include/

#build programs
buildUserspace init
cp init/init $SYSROOT

buildUserspace sh

#build default ports
cd ports
for port in $(cat default_ports); do
	make -C $port
	#make -C $port clean
done
cd ..

#create initrd
cd $INITRDDIR
find * -type f | cpio --create --format=newc > ../$BUILDDIR/initrd

cd ..

#create iso
if [ "$EFI" = true ]; then
	xorrisofs -r -J -o out.iso build/
else
	mkdir -p $BUILDDIR/grub
	cp grub.cfg $BUILDDIR/grub/
	grub-mkrescue -o out.iso build/
fi