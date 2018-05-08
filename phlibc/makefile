OUTPUT = libc.a

OBJECTS = crt0.o syscalls.o \
assert.o basicio.o printf.o string.o exec.o stdlib.o mem.o bufio.o signal.o setjmp.o floatops.o popen.o time.o

SRC = src

LIBDIR = $(SYSROOT)/lib

$(shell mkdir -p $(DEPDIR) > /dev/null)

FLAG_INCLUDES = -Iinclude/
CFLAG = $(CFLAG_USER) $(FLAG_DEP) $(FLAG_INCLUDES) -c
CC := $(TARGET_USER)-gcc

all: $(LIBDIR)/libc.a $(LIBDIR)/crt0.o

clean:
	rm -f $(OUTPUT) $(OBJECTS) $(SRC)/syscalls.asm

$(LIBDIR)/libc.a: $(OBJECTS)
	@echo "(AR)		$@"
	@$(TARGET_USER)-ar rcs $@ $+

$(LIBDIR)/crt0.o: crt0.o
	cp $< $@

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

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(obj-y)) ))