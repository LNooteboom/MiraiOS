sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/smpcall.o $(d)/spinlock.o $(d)/thread.o $(d)/threadhelpers.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))