sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/main.o $(d)/ramfs.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))