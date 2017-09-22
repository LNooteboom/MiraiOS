sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/init.o $(d)/map.o $(d)/pagefault.o $(d)/tlb.o $(d)/virtpaging.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))