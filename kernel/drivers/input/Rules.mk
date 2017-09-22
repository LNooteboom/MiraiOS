sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

$(shell mkdir -p $(DEPDIR)/$(d)/ps2 > /dev/null)

obj-$(CONFIG_I8042) += $(d)/ps2/i8042.o
obj-$(CONFIG_KEYBOARD) += $(d)/ps2/keyboard.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))