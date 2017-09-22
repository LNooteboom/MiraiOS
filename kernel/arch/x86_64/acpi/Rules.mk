sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/acpi.o $(d)/rsdp.o $(d)/madt.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))