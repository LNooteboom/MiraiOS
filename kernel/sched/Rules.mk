sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/lock.o $(d)/queue.o $(d)/readyqueue.o $(d)/sleep.o $(d)/thread.o

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))