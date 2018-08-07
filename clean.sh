export SYSROOT=$PWD/sysroot

make -C kernel clean
make -C init clean
make -C phlibc clean
make -C sh clean

#rm -rf initrd/
rm -rf build/
rm -rf sysroot/*

cd ports
for port in $(cat default_ports); do
	make -C $port clean
done
cd ..