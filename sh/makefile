d = $(SYSROOT)/bin
PROGRAMS = $(d)/sh $(d)/echo $(d)/ls

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS)

$(d)/%: %.c
	@echo "(USER-CC)	$@"
	@$(TARGET_USER)-gcc $(CFLAG_USER) -o $@ $+ -lopenlibm