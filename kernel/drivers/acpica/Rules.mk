sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

export ACPICA_CFLAGS=$(FLAG_WARNINGS) $(FLAG_FREESTANDING) $(FLAG_KERNEL) $(FLAG_MEMMODEL) $(FLAG_INCLUDES) $(FLAG_DEBUG) $(FLAG_OPTIMIZE) $(FLAG_NOFLOAT) -D__miraios__

obj-y += $(d)/acpica.o

$(d)/acpica.o: $(d)/getacpica.sh $(d)/acmirai.h
	sh $+

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))