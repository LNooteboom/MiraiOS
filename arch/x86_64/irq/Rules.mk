sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/entry.o $(d)/exception.o $(d)/idt.o $(d)/ioapic.o $(d)/main.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))