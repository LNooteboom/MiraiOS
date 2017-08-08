sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/main.o $(d)/dirops.o $(d)/fileops.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))