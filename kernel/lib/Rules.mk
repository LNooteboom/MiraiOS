sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/rbtree.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))