ACPICA_VERSION=20190215

cd "$(dirname "$0")"

if [ ! -f acpica-unix-${ACPICA_VERSION}.tar.gz ]; then
	wget https://acpica.org/sites/acpica/files/acpica-unix-${ACPICA_VERSION}.tar.gz
fi

tar -xzf acpica-unix-${ACPICA_VERSION}.tar.gz
mkdir -p src
#cp acpica-unix-${ACPICA_VERSION}/source/components/debugger/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/dispatcher/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/events/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/executer/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/hardware/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/namespace/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/parser/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/resources/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/tables/* src/
cp acpica-unix-${ACPICA_VERSION}/source/components/utilities/* src/
cp -r acpica-unix-${ACPICA_VERSION}/source/include/* src/
rm src/rsdump.c

awk '/#elif defined\(_APPLE\)*/{print "#elif defined(__miraios__)\n#include \"../../acmirai.h\""}{print $0}'\
	acpica-unix-${ACPICA_VERSION}/source/include/platform/acenv.h > src/platform/acenv.h


for filename in src/*.c; do
	#echo $filename  
	echo "(CC) $filename.o"
	${CC} ${ACPICA_CFLAGS} -Werror -Wno-unused-parameter -o $filename.o -c $filename
	OUT=$?
	if [ ! $OUT -eq 0 ]; then
		exit $OUT
	fi
	FILES="$FILES $filename.o"
done
#echo ${ACPICA_CFLAGS}
ld -r -o acpica.o $FILES