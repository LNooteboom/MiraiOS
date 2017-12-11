
.PHONY = all clean

all: init test

init: main.o
	x86_64-elf-ld -o $@ main.o

test: test.o
	x86_64-elf-ld -o $@ test.o

%.o: %.asm
	nasm -f elf64 -g -F dwarf -o $@ $<
	
clean:
	rm -f init test test.o main.o