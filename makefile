TARGET := x86_64-elf
ARCH := x86_64

KERNEL = miraiBoot

KERNEL_ROOT := .
DEPDIR := .d
$(shell mkdir -p $(DEPDIR) > /dev/null)

FLAG_WARNINGS := -Wall -Wextra
FLAG_FREESTANDING := -ffreestanding -nostdlib -nostartfiles -fno-pie
FLAG_KERNEL := -masm=intel -mno-red-zone -mcmodel=kernel
FLAG_INCLUDES := -I$(KERNEL_ROOT)/include/ -I$(KERNEL_ROOT)/arch/${ARCH}/include
FLAG_DEBUG := -g
FLAG_OPTIMIZE := -O2
FLAG_DEP = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

CFLAG = $(FLAG_WARNINGS) $(FLAG_FREESTANDING) $(FLAG_KERNEL) $(FLAG_INCLUDES) $(FLAG_DEBUG) $(FLAG_OPTIMIZE) $(FLAG_DEP) -c
CC := $(TARGET)-gcc

NASM := nasm
NASMFLAG := -f elf64
LD := $(TARGET)-ld

obj-y :=

include config

.PHONY: all clean

all: $(KERNEL)

clean:
	rm -rf $(obj-y) $(DEPDIR)

#subdirs
dir := $(KERNEL_ROOT)/kernel
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

dir := $(KERNEL_ROOT)/drivers
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

$(KERNEL): $(obj-y)
	@echo "(LD) $@"
	@$(LD) -z max-page-size=0x1000 -T kernel.ld $(obj-y)

%.o: %.c
%.o: %.c $(DEPDIR)/%.d
	@echo "(CC) $@"
	@${CC} ${CFLAG} -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

%.o: %.asm
	@echo "(NASM) $@"
	@${NASM} ${NASMFLAG} -o $@ $<

%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(obj-y)) ))