#!/bin/bash

buildUserspace () {
	if [ -f "$1/.buildtime" ] && [ $SYSROOT/phlibc/libc.a -nt $1/.buildtime ]; then
		make -C $1 clean
	fi
	make -C $1
	OUT=$?
	touch $1/.buildtime
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
}

#config options
export TARGET_KERNEL=x86_64-elf
export TARGET_USER=x86_64-miraios

BUILDDIR=build;

export INITRDDIR=$PWD/initrd;
export SYSROOT=$PWD/sysroot

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
cp phlibc/libc.a $SYSROOT/lib/
cp phlibc/crt0.o $SYSROOT/lib/
cp -r phlibc/userinclude/* $SYSROOT/include/

#build programs
buildUserspace init
cp init/init $SYSROOT

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