sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/main.o $(d)/print.o $(d)/panic.o $(d)/irq.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))