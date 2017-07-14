sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

dir := $(d)/input
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

$(shell mkdir -p $(DEPDIR)/$(d)/vga > /dev/null)

#legacy vga drivers will be removed soon
obj-y += $(d)/vga/crtc.o $(d)/vga/tty.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))