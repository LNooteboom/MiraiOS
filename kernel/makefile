TARGET := x86_64-elf
ARCH := x86_64

KERNEL = miraiBoot

KERNEL_ROOT := .
DEPDIR := .d
$(shell mkdir -p $(DEPDIR) > /dev/null)

FLAG_WARNINGS := -Wall -Wextra
FLAG_FREESTANDING := -ffreestanding -nostdlib -nostartfiles -fno-pie
FLAG_KERNEL := -masm=intel -mno-red-zone -fshort-wchar
FLAG_MEMMODEL := -mcmodel=kernel
FLAG_INCLUDES := -I$(KERNEL_ROOT)/include/ -I$(KERNEL_ROOT)/arch/${ARCH}/include
FLAG_DEBUG := -g
FLAG_OPTIMIZE := -O2 -pipe
FLAG_DEP = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
FLAG_NOFLOAT = -mno-mmx -mno-sse -mno-sse2

CFLAG = $(FLAG_WARNINGS) $(FLAG_FREESTANDING) $(FLAG_KERNEL) $(FLAG_MEMMODEL) $(FLAG_INCLUDES) $(FLAG_DEBUG) $(FLAG_OPTIMIZE) $(FLAG_DEP) $(FLAG_NOFLOAT) -c
CC := $(TARGET)-gcc

NASM := nasm
NASMFLAG := -f elf64 -g -F dwarf
LD := $(TARGET)-ld

obj-y :=

include config

.PHONY: all clean

ifeq ($(CONFIG_EFI_STUB),y)
EFIBUILD := efibuild

all: $(KERNEL).efi

$(KERNEL).efi: $(KERNEL) $(EFIBUILD)
	@echo "(EFIBUILD) $@"
	@./$(EFIBUILD) $(KERNEL) $@

clean:
	rm -rf $(obj-y) $(DEPDIR) $(KERNEL) $(KERNEL).efi $(EFIBUILD)
else
all: $(KERNEL)

clean:
	rm -rf $(obj-y) $(DEPDIR) $(KERNEL)
endif

#subdirs
dir := $(KERNEL_ROOT)/main
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(KERNEL_ROOT)/arch/$(ARCH)
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(KERNEL_ROOT)/mm
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(KERNEL_ROOT)/sched
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(KERNEL_ROOT)/fs
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(KERNEL_ROOT)/drivers
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(KERNEL_ROOT)/lib
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

$(KERNEL): $(obj-y) $(tools) $(LDSCRIPT) config
	@echo "(LD) $@"
	@$(LD) -z max-page-size=0x1000 -T $(LDSCRIPT) $(obj-y)

%.o: %.c
%.o: %.c $(DEPDIR)/%.d
	@echo "(CC)	$@"
	@${CC} ${CFLAG} -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

%.o: %.asm
	@echo "(NASM)	$@"
	@${NASM} ${NASMFLAG} -o $@ $<

%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(obj-y)) ))