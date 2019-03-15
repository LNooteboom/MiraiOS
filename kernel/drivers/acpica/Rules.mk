sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

ifeq ($(CONFIG_ACPICA),y)
$(d)/shutdown.o: $(d)/acpica.o
endif

export ACPICA_CFLAGS=$(FLAG_WARNINGS) $(FLAG_FREESTANDING) $(FLAG_KERNEL) $(FLAG_MEMMODEL) $(FLAG_INCLUDES) $(FLAG_DEBUG) $(FLAG_OPTIMIZE) $(FLAG_NOFLOAT)

obj-$(CONFIG_ACPICA) += $(d)/acpica.o $(d)/osl.o $(d)/main.o
obj-y += $(d)/shutdown.o

$(d)/acpica.o: $(d)/getacpica.sh $(d)/acmirai.h
	sh $+

$(d)/osl.o: $(d)/acpica.o

$(d)/main.o: $(d)/acpica.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))