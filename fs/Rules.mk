sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

dir := $(d)/ramfs
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

obj-y += $(d)/main.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))