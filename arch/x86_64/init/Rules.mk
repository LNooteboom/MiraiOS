sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/boot.o $(d)/init.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))