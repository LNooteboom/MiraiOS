
.PHONY = all clean

all: init

init: main.asm
	nasm -f bin -o $@ $<
	
clean:
	rm init