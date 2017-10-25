
.PHONY = all clean

all: init

init: main.asm
	nasm -f elf64 -o main.o $<
	x86_64-elf-ld -o $@ main.o
	
clean:
	rm init