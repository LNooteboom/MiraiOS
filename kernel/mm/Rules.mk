sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/heap.o $(d)/init.o $(d)/lowmem.o $(d)/physpaging.o $(d)/mmap.o $(d)/cow.o $(d)/cache.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))