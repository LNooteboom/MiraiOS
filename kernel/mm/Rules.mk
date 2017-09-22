sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/heap.o $(d)/init.o $(d)/lowmem.o $(d)/physpaging.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))