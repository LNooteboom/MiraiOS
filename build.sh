#!/bin/bash

BUILDDIR=build;
INITRDDIR=initrd;

KERNELDIR=kernel;
KERNELNAME=miraiBoot;

EFI=false;

mkdir -p $BUILDDIR
mkdir -p $INITRDDIR

#make kernel
make -j 4 -C $KERNELDIR
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

make -C init
cp init/init $INITRDDIR

#create initrd
cd $INITRDDIR
echo "init" | cpio --create --format=newc > ../$BUILDDIR/initrd

cd ..

#create iso
if [ "$EFI" = true ]; then
	xorrisofs -r -J -o out.iso build/
else
	mkdir -p $BUILDDIR/grub
	cp grub.cfg $BUILDDIR/grub/
	grub-mkrescue -o out.iso build/
fi