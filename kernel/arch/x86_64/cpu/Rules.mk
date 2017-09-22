sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

obj-y += $(d)/lapic.o $(d)/pcpu.o $(d)/smpboot.o $(d)/tssgdt.o

$(d)/smpboot.o: $(d)/smpboot16.bin

$(d)/smpboot16.bin: $(d)/smpboot16.asm
	@$(NASM) -f bin -o $@ $<

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))