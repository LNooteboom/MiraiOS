OUTPUT = libc.a

OBJECTS = crt0.o syscalls.o \
assert.o basicio.o printf.o string.o exec.o stdlib.o

SRC = src

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) > /dev/null)

FLAG_INCLUDES = -Iinclude/ -I$(HOME)/opt/cross/lib/gcc/x86_64-miraios/7.3.0/include -I../kernel/include
FLAG_DEP = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
CFLAG = $(FLAG_DEP) $(FLAG_INCLUDES) -Wall -Wextra -O2 -std=gnu99  -nostdinc -c
CC := $(TARGET_USER)-gcc

all: libc.a crt0.o

clean:
	rm -f $(OUTPUT) $(OBJECTS)

libc.a: $(OBJECTS)
	$(TARGET_USER)-ar rcs $@ $+

%.o: %.c
%.o: $(SRC)/%.c $(DEPDIR)/%.d
	@echo "(USER-CC)	$@"
	@${CC} ${CFLAG} -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

%.o: $(SRC)/%.asm
	@echo "(USER-NASM)	$@"
	@nasm -f elf64 -g -F dwarf -o $@ $<

$(SRC)/syscalls.asm: $(SRC)/gensyscalls.awk ../kernel/include/uapi/syscalls.h
	awk -f $+ > $@

%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(obj-y)) ))