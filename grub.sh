GRUB_VERSION=2.02

KROOT=$PWD

cd cross

if [ ! -f grub-${GRUB_VERSION}.tar.xz ]; then
	wget ftp://ftp.gnu.org/gnu/grub/grub-2.02.tar.xz
fi

tar -xJf grub-${GRUB_VERSION}.tar.xz
cd grub-${GRUB_VERSION}
./configure --prefix=${KROOT}/cross --target=i386 --with-platform=pc --disable-efiemu --disable-werror
make -j 4
make install

