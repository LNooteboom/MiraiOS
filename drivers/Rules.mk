sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

dir := $(d)/video
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(d)/input
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

$(shell mkdir -p $(DEPDIR)/$(d)/vga > /dev/null)

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))