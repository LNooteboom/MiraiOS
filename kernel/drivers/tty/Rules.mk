sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/font8x16.o $(d)/framebuffer.o $(d)/main.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))