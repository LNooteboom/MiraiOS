DEPDIR := .d
$(shell mkdir -p $(DEPDIR) > /dev/null)

CC = $(TARGET_USER)-gcc

FLAG_WARN = -Wall -Wextra
FLAG_DEP = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
CFLAG = $(FLAG_WARN) $(FLAG_DEP) -c

LIBS = 
LD = $(CC)

OUTPUT = init
OBJECTS = main.o

.PHONY: all clean

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT) $(OBJECTS)

$(OUTPUT): $(OBJECTS)
	@echo "(LD) $@"
	@$(LD) -o $@ $+ $(LIBS)

%.o: %.c
%.o: %.c $(DEPDIR)/%.d
	@echo "(CC)	$@"
	@${CC} ${CFLAG} -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(OBJECTS)) ))