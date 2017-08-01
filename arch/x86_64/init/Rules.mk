sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

ifeq ($(CONFIG_EFI_STUB),y)

$(shell mkdir -p $(DEPDIR)/$(d)/efi > /dev/null)

obj-y += $(d)/efi/init.o $(d)/efi/main.o $(d)/efi/efiCall.o $(d)/efi/initrd.o

$(EFIBUILD): $(d)/efi/tools/build.c
	@echo "(HOSTCC) $@"
	@gcc -std=gnu99 -O2 -Wall -o $@ $<

LDSCRIPT := $(d)/efi/efikernel.ld

else

obj-y += $(d)/boot.o $(d)/init.o

LDSCRIPT := $(d)/multiboot.ld

endif

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))