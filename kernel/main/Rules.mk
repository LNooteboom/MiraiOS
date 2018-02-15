sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/main.o $(d)/print.o $(d)/panic.o $(d)/irq.o $(d)/exec.o $(d)/fork.o $(d)/exit.o $(d)/syscalltbl.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))