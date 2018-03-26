d = $(SYSROOT)/bin
PROGRAMS = $(d)/sh $(d)/echo $(d)/ls

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

$(d)/%: %.c
	$(TARGET_USER)-gcc $(CFLAG_USER) -o $@ $+