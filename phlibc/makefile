OUTPUT = libc.a
OBJECTS = crt0.o syscalls.o

all: libc.a crt0.o

clean:
	rm -f $(OUTPUT) $(OBJECTS)

libc.a: $(OBJECTS)
	$(TARGET_USER)-ar rcs $@ $+

%.o: %.asm
	nasm -f elf64 -g -F dwarf -o $@ $<

syscalls.asm: gensyscalls.awk ../kernel/include/syscalls.h
	awk -f $+ > $@