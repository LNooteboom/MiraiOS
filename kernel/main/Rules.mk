sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/main.o $(d)/print.o $(d)/panic.o $(d)/irq.o $(d)/exec.o $(d)/fork.o $(d)/exit.o $(d)/syscalltbl.o

$(d)/syscalltbl.c: $(d)/syscalltbl.awk $(KERNEL_ROOT)/include/uapi/syscalls.h
	@echo "(AWK)	$@"
	@awk -f $+ > $@

clean: $(d)/syscalltbl.c

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))