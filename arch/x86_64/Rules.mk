sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

dir := $(d)/acpi
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(d)/cpu
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(d)/init
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(d)/irq
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(d)/mm
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

dir := $(d)/sched
$(shell mkdir -p $(DEPDIR)/$(dir) > /dev/null)
include $(dir)/Rules.mk

obj-y += $(d)/main.o $(d)/i8253.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))