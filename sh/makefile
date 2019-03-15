d = $(SYSROOT)/bin
PROGRAMS = $(d)/sh $(d)/echo $(d)/ls $(d)/touch $(d)/rm $(d)/mkdir $(d)/cat $(d)/shutdown

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

$(d)/%: %.c
	@echo "(USER-CC)	$@"
	@$(TARGET_USER)-gcc $(CFLAG_USER) -o $@ $+