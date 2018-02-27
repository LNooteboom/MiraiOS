
make -C kernel clean
make -C init clean
make -C phlibc clean

#rm -rf initrd/
rm -rf build/
rm -rf sysroot/*